#include "AdvancedCommandRecorder.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "Descriptor/DescriptorPool.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/ThreadPool.hpp"
#include "../scene/Scene.hpp"
#include "../scene/GameObject.hpp"
#include "../ui/ImGuiManager.hpp"
#include <stdexcept>
#include <iostream>
#include <array>
#include <chrono>

AdvancedCommandRecorder::AdvancedCommandRecorder(Device *device, ThreadPool *threadPool, uint32_t framesInFlight, Pipeline* gizmoPipeline, Pipeline* gridPipeline)
    : m_device(device), m_threadPool(threadPool), m_framesInFlight(framesInFlight), m_threadCount(static_cast<uint32_t>(threadPool->getThreadCount())), m_gizmoPipeline(gizmoPipeline), m_gridPipeline(gridPipeline)
{
    createCommandPools();
    createCommandBuffers();

    std::cout << "✓ Advanced Command Recorder created ("
              << m_threadCount << " threads, "
              << m_framesInFlight << " frames)" << std::endl;
}

AdvancedCommandRecorder::~AdvancedCommandRecorder()
{
    VkDevice device = m_device->getLogicalDevice();

    // Détruire les secondary command buffers
    for (size_t i = 0; i < m_secondaryCommandPools.size(); i++)
    {
        if (!m_secondaryCommandBuffers.empty())
        {
            std::vector<VkCommandBuffer> buffersForPool;
            for (uint32_t f = 0; f < m_framesInFlight; f++)
            {
                buffersForPool.push_back(m_secondaryCommandBuffers[getSecondaryBufferIndex(f, i)]);
            }
            vkFreeCommandBuffers(device, m_secondaryCommandPools[i],
                                 static_cast<uint32_t>(buffersForPool.size()),
                                 buffersForPool.data());
        }
        vkDestroyCommandPool(device, m_secondaryCommandPools[i], nullptr);
    }
    
    // Détruire les ImGui command buffers
    if (!m_imguiCommandBuffers.empty())
    {
        vkFreeCommandBuffers(device, m_imguiCommandPool,
                            static_cast<uint32_t>(m_imguiCommandBuffers.size()),
                            m_imguiCommandBuffers.data());
    }
    if (m_imguiCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, m_imguiCommandPool, nullptr);
    }

    // Détruire les primary command buffers
    if (!m_primaryCommandBuffers.empty())
    {
        vkFreeCommandBuffers(device, m_primaryCommandPool,
                             static_cast<uint32_t>(m_primaryCommandBuffers.size()),
                             m_primaryCommandBuffers.data());
    }

    if (m_primaryCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, m_primaryCommandPool, nullptr);
    }
}

void AdvancedCommandRecorder::createCommandPools()
{
    QueueFamilyIndices queueFamilies = m_device->getQueueFamilies();

    // Primary command pool
    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    primaryPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

    if (vkCreateCommandPool(m_device->getLogicalDevice(), &primaryPoolInfo, nullptr, &m_primaryCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create primary command pool!");
    }

    // Secondary command pools (un par thread)
    m_secondaryCommandPools.resize(m_threadCount);

    for (uint32_t i = 0; i < m_threadCount; i++)
    {
        VkCommandPoolCreateInfo secondaryPoolInfo{};
        secondaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        secondaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        secondaryPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

        if (vkCreateCommandPool(m_device->getLogicalDevice(), &secondaryPoolInfo, nullptr, &m_secondaryCommandPools[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create secondary command pool!");
        }
    }
    
    // ImGui command pool (séparé, un seul pool partagé)
    VkCommandPoolCreateInfo imguiPoolInfo{};
    imguiPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    imguiPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    imguiPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

    if (vkCreateCommandPool(m_device->getLogicalDevice(), &imguiPoolInfo, nullptr, &m_imguiCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create ImGui command pool!");
    }
}

void AdvancedCommandRecorder::createCommandBuffers()
{
    // Primary command buffers
    m_primaryCommandBuffers.resize(m_framesInFlight);

    VkCommandBufferAllocateInfo primaryAllocInfo{};
    primaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    primaryAllocInfo.commandPool = m_primaryCommandPool;
    primaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    primaryAllocInfo.commandBufferCount = m_framesInFlight;

    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &primaryAllocInfo, m_primaryCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate primary command buffers!");
    }

    // Secondary command buffers (threadCount x framesInFlight)
    m_secondaryCommandBuffers.resize(m_threadCount * m_framesInFlight);

    for (uint32_t t = 0; t < m_threadCount; t++)
    {
        std::vector<VkCommandBuffer> buffersForThread(m_framesInFlight);

        VkCommandBufferAllocateInfo secondaryAllocInfo{};
        secondaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        secondaryAllocInfo.commandPool = m_secondaryCommandPools[t];
        secondaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        secondaryAllocInfo.commandBufferCount = m_framesInFlight;

        if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &secondaryAllocInfo, buffersForThread.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate secondary command buffers!");
        }

        for (uint32_t f = 0; f < m_framesInFlight; f++)
        {
            m_secondaryCommandBuffers[getSecondaryBufferIndex(f, t)] = buffersForThread[f];
        }
    }
    
    // ImGui command buffers (un par frame, secondary level)
    m_imguiCommandBuffers.resize(m_framesInFlight);

    VkCommandBufferAllocateInfo imguiAllocInfo{};
    imguiAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    imguiAllocInfo.commandPool = m_imguiCommandPool;
    imguiAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    imguiAllocInfo.commandBufferCount = m_framesInFlight;

    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &imguiAllocInfo, m_imguiCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate ImGui command buffers!");
    }
}

void AdvancedCommandRecorder::record(
    uint32_t frameIndex,
    uint32_t imageIndex,
    RenderPass *renderPass,
    Pipeline *pipeline,
    SwapChain *swapChain,
    DescriptorPool *descriptorPool,
    Scene *scene,
    ImGuiManager *imguiManager)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    size_t totalObjects = scene->getObjectCount();
    m_stats.objectsRendered = static_cast<uint32_t>(totalObjects);

    if (totalObjects == 0)
    {
        // Enregistrer un command buffer vide
        VkCommandBuffer primaryCmd = m_primaryCommandBuffers[frameIndex];
        vkResetCommandBuffer(primaryCmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(primaryCmd, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass->getHandle();
        renderPassInfo.framebuffer = swapChain->getFramebuffers()[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain->getExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues.data();

        // Utiliser INLINE pour pouvoir mélanger secondary buffers et commandes directes (ImGui)
        vkCmdBeginRenderPass(primaryCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        // Render ImGui même si pas d'objets
        if (imguiManager)
        {
            imguiManager->beginFrame();
            imguiManager->endFrame(primaryCmd);
        }
        
        vkCmdEndRenderPass(primaryCmd);
        vkEndCommandBuffer(primaryCmd);

        m_stats.threadsUsed = 0;
        m_stats.lastRecordTime = 0.0f;
        return;
    }

    // Calculer la répartition des objets pour le multi-threading
    uint32_t objectsPerThread = static_cast<uint32_t>((totalObjects + m_threadCount - 1) / m_threadCount);
    uint32_t threadsNeeded = static_cast<uint32_t>((totalObjects + objectsPerThread - 1) / objectsPerThread);
    uint32_t threadsToUse = std::min(threadsNeeded, m_threadCount);

    m_stats.threadsUsed = threadsToUse;

    // Créer les tâches pour le thread pool
    std::vector<std::function<void()>> tasks;
    tasks.reserve(threadsToUse);

    for (uint32_t t = 0; t < threadsToUse; t++)
    {
        uint32_t objectStart = t * objectsPerThread;
        uint32_t objectCount = std::min(objectsPerThread, static_cast<uint32_t>(totalObjects - objectStart));

        if (objectCount == 0)
            break;

        RenderTask task{};
        task.threadIndex = t;
        task.frameIndex = frameIndex;
        task.objectStart = objectStart;
        task.objectCount = objectCount;
        task.commandBuffer = m_secondaryCommandBuffers[getSecondaryBufferIndex(frameIndex, t)];
        task.renderPass = renderPass;
        task.pipeline = pipeline;
        task.swapChain = swapChain;
        task.descriptorPool = descriptorPool;
        task.scene = scene;

        // Si le batch contient le gizmo, utiliser le pipeline unlit
        const auto &objects = scene->getObjects();
        for (uint32_t i = objectStart; i < objectStart + objectCount && i < objects.size(); i++) {
            if (objects[i]->getName() == "AxisGizmo") {
                task.pipeline = m_gizmoPipeline;
                break;
            }
        }

        tasks.push_back([this, task]()
                        { recordSecondaryCommandBuffer(task); });
    }

    // Exécuter toutes les tâches en parallèle
    m_threadPool->parallelFor(tasks);

    // Enregistrer ImGui dans son propre secondary command buffer
    if (imguiManager)
    {
        VkCommandBuffer imguiCmd = m_imguiCommandBuffers[frameIndex];
        vkResetCommandBuffer(imguiCmd, 0);

        VkCommandBufferInheritanceInfo inheritanceInfo{};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = renderPass->getHandle();
        inheritanceInfo.subpass = 0;
        inheritanceInfo.framebuffer = VK_NULL_HANDLE;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        if (vkBeginCommandBuffer(imguiCmd, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin ImGui command buffer!");
        }

        // Enregistrer ImGui
        imguiManager->beginFrame();
        imguiManager->endFrame(imguiCmd);

        if (vkEndCommandBuffer(imguiCmd) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end ImGui command buffer!");
        }
    }

    // Enregistrer le primary command buffer directement (sans secondary buffers)
    VkCommandBuffer primaryCmd = m_primaryCommandBuffers[frameIndex];
    vkResetCommandBuffer(primaryCmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(primaryCmd, &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = swapChain->getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues.data();

    // Utiliser SECONDARY_COMMAND_BUFFERS car on va exécuter les secondary buffers
    vkCmdBeginRenderPass(primaryCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    // Exécuter les secondary command buffers de la scène 3D
    std::vector<VkCommandBuffer> secondaryBuffers;
    secondaryBuffers.reserve(threadsToUse + 1); // +1 pour ImGui

    for (uint32_t t = 0; t < threadsToUse; t++)
    {
        secondaryBuffers.push_back(m_secondaryCommandBuffers[getSecondaryBufferIndex(frameIndex, t)]);
    }

    // Ajouter le command buffer ImGui à la fin
    if (imguiManager)
    {
        secondaryBuffers.push_back(m_imguiCommandBuffers[frameIndex]);
    }

    if (!secondaryBuffers.empty())
    {
        vkCmdExecuteCommands(primaryCmd, static_cast<uint32_t>(secondaryBuffers.size()), secondaryBuffers.data());
    }

    vkCmdEndRenderPass(primaryCmd);
    vkEndCommandBuffer(primaryCmd);

    // Calculer les stats
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.lastRecordTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
}

// Cette fonction n'est plus utilisée avec le mode INLINE, mais gardée pour référence future
// si on veut réimplémenter le multi-threading
void AdvancedCommandRecorder::recordSecondaryCommandBuffer(const RenderTask &task)
{
    VkCommandBuffer cmdBuffer = task.commandBuffer;

    vkResetCommandBuffer(cmdBuffer, 0);

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = task.renderPass->getHandle();
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin secondary command buffer!");
    }


    // Viewport et scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(task.swapChain->getExtent().width);
    viewport.height = static_cast<float>(task.swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = task.swapChain->getExtent();
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // Binder descriptor set
    VkDescriptorSet descriptorSet = task.descriptorPool->getDescriptorSet(task.frameIndex);

    // Dessiner la grille d'abord si présente
    const auto &objects = task.scene->getObjects();
    for (uint32_t i = task.objectStart; i < std::min(task.objectStart + task.objectCount, static_cast<uint32_t>(objects.size())); i++) {
        if (objects[i]->getName() == "GridXZ") {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_gridPipeline->getHandle());
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   m_gridPipeline->getLayout(), 0, 1, &descriptorSet, 0, nullptr);
            objects[i]->draw(cmdBuffer, m_gridPipeline->getLayout());
        }
    }
    // Puis dessiner les autres objets
    for (uint32_t i = task.objectStart; i < std::min(task.objectStart + task.objectCount, static_cast<uint32_t>(objects.size())); i++) {
        if (objects[i]->getName() != "GridXZ") {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, task.pipeline->getHandle());
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   task.pipeline->getLayout(), 0, 1, &descriptorSet, 0, nullptr);
            objects[i]->draw(cmdBuffer, task.pipeline->getLayout());
        }
    }

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to end secondary command buffer!");
    }
}