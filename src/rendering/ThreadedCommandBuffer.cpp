#include "ThreadedCommandBuffer.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "Descriptor/DescriptorPool.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../scene/Scene.hpp"
#include "../scene/GameObject.hpp"
#include <stdexcept>
#include <iostream>
#include <array>
#include <algorithm>

ThreadedCommandBuffer::ThreadedCommandBuffer(Device* device, uint32_t threadCount, uint32_t framesInFlight)
    : m_device(device)
    , m_threadCount(threadCount)
    , m_framesInFlight(framesInFlight)
    , m_primaryCommandPool(VK_NULL_HANDLE)
{
    createCommandPools();
    createCommandBuffers();
    
    std::cout << "✓ Threaded command buffers created (" 
              << m_threadCount << " threads, " 
              << m_framesInFlight << " frames in flight)" << std::endl;
}

ThreadedCommandBuffer::~ThreadedCommandBuffer() {
    VkDevice device = m_device->getLogicalDevice();

    // Détruire les command buffers secondaires
    for (size_t i = 0; i < m_secondaryCommandPools.size(); i++) {
        if (!m_secondaryCommandBuffers. empty()) {
            std::vector<VkCommandBuffer> buffersForPool;
            for (uint32_t f = 0; f < m_framesInFlight; f++) {
                buffersForPool. push_back(m_secondaryCommandBuffers[getSecondaryBufferIndex(f, i)]);
            }
            vkFreeCommandBuffers(device, m_secondaryCommandPools[i], 
                               static_cast<uint32_t>(buffersForPool.size()), 
                               buffersForPool. data());
        }
        vkDestroyCommandPool(device, m_secondaryCommandPools[i], nullptr);
    }

    // Détruire les command buffers primaires
    if (!m_primaryCommandBuffers. empty()) {
        vkFreeCommandBuffers(device, m_primaryCommandPool, 
                           static_cast<uint32_t>(m_primaryCommandBuffers.size()), 
                           m_primaryCommandBuffers.data());
    }
    
    if (m_primaryCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, m_primaryCommandPool, nullptr);
    }
}

void ThreadedCommandBuffer::createCommandPools() {
    QueueFamilyIndices queueFamilies = m_device->getQueueFamilies();

    // Créer le command pool PRIMAIRE
    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    primaryPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

    if (vkCreateCommandPool(m_device->getLogicalDevice(), &primaryPoolInfo, nullptr, &m_primaryCommandPool) != VK_SUCCESS) {
        throw std:: runtime_error("Failed to create primary command pool!");
    }

    // Créer un command pool SECONDAIRE par thread
    m_secondaryCommandPools.resize(m_threadCount);
    
    for (uint32_t i = 0; i < m_threadCount; i++) {
        VkCommandPoolCreateInfo secondaryPoolInfo{};
        secondaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        secondaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        secondaryPoolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

        if (vkCreateCommandPool(m_device->getLogicalDevice(), &secondaryPoolInfo, nullptr, &m_secondaryCommandPools[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create secondary command pool!");
        }
    }
}

void ThreadedCommandBuffer::createCommandBuffers() {
    // Allouer les command buffers PRIMAIRES (un par frame)
    m_primaryCommandBuffers.resize(m_framesInFlight);

    VkCommandBufferAllocateInfo primaryAllocInfo{};
    primaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    primaryAllocInfo.commandPool = m_primaryCommandPool;
    primaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    primaryAllocInfo.commandBufferCount = m_framesInFlight;

    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &primaryAllocInfo, m_primaryCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate primary command buffers!");
    }

    // Allouer les command buffers SECONDAIRES
    // Total : threadCount x framesInFlight
    m_secondaryCommandBuffers.resize(m_threadCount * m_framesInFlight);

    for (uint32_t t = 0; t < m_threadCount; t++) {
        std::vector<VkCommandBuffer> buffersForThread(m_framesInFlight);

        VkCommandBufferAllocateInfo secondaryAllocInfo{};
        secondaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        secondaryAllocInfo.commandPool = m_secondaryCommandPools[t];
        secondaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        secondaryAllocInfo.commandBufferCount = m_framesInFlight;

        if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &secondaryAllocInfo, buffersForThread.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate secondary command buffers!");
        }

        // Copier dans le vecteur principal
        for (uint32_t f = 0; f < m_framesInFlight; f++) {
            m_secondaryCommandBuffers[getSecondaryBufferIndex(f, t)] = buffersForThread[f];
        }
    }
}

void ThreadedCommandBuffer::record(uint32_t frameIndex, uint32_t imageIndex, RenderPass* renderPass,
                                  Pipeline* pipeline, SwapChain* swapChain, DescriptorPool* descriptorPool,
                                  Scene* scene) {
    // Diviser les objets entre les threads
    size_t totalObjects = scene->getObjectCount();
    
    if (totalObjects == 0) {
        return;  // Rien à rendre
    }

    uint32_t objectsPerThread = static_cast<uint32_t>((totalObjects + m_threadCount - 1) / m_threadCount);

    // Lancer les threads pour enregistrer les command buffers secondaires
    std::vector<std::thread> threads;
    threads.reserve(m_threadCount);

    for (uint32_t t = 0; t < m_threadCount; t++) {
        uint32_t objectStart = t * objectsPerThread;
        uint32_t objectCount = std::min(objectsPerThread, static_cast<uint32_t>(totalObjects - objectStart));

        if (objectCount == 0) {
            break;  // Plus d'objets à rendre
        }

        ThreadData data{};
        data.threadIndex = t;
        data.frameIndex = frameIndex;
        data.objectStart = objectStart;
        data.objectCount = objectCount;
        data.commandBuffer = m_secondaryCommandBuffers[getSecondaryBufferIndex(frameIndex, t)];
        data.renderPass = renderPass;
        data.pipeline = pipeline;
        data.swapChain = swapChain;
        data.descriptorPool = descriptorPool;
        data.scene = scene;

        threads.emplace_back([this, data]() {
            recordSecondaryCommandBuffer(data);
        });
    }

    // Attendre que tous les threads finissent
    for (auto& thread : threads) {
        thread.join();
    }

    // Enregistrer le command buffer PRIMAIRE qui exécute les secondaires
    VkCommandBuffer primaryCmdBuffer = m_primaryCommandBuffers[frameIndex];

    vkResetCommandBuffer(primaryCmdBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo. sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(primaryCmdBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording primary command buffer!");
    }

    // Commencer le render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = swapChain->getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(primaryCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    // Exécuter tous les command buffers secondaires
    uint32_t activeThreads = std::min(m_threadCount, static_cast<uint32_t>((totalObjects + objectsPerThread - 1) / objectsPerThread));
    std::vector<VkCommandBuffer> secondaryBuffers(activeThreads);
    
    for (uint32_t t = 0; t < activeThreads; t++) {
        secondaryBuffers[t] = m_secondaryCommandBuffers[getSecondaryBufferIndex(frameIndex, t)];
    }

    vkCmdExecuteCommands(primaryCmdBuffer, activeThreads, secondaryBuffers.data());

    vkCmdEndRenderPass(primaryCmdBuffer);

    if (vkEndCommandBuffer(primaryCmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record primary command buffer!");
    }
}

void ThreadedCommandBuffer::recordSecondaryCommandBuffer(const ThreadData& data) {
    VkCommandBuffer cmdBuffer = data.commandBuffer;

    vkResetCommandBuffer(cmdBuffer, 0);

    // Info d'héritage pour les command buffers secondaires
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = data.renderPass->getHandle();
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = VK_NULL_HANDLE;  // Sera spécifié dans le primaire

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording secondary command buffer!");
    }

    // Binder le pipeline
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline->getHandle());

    // Viewport et scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(data.swapChain->getExtent().width);
    viewport.height = static_cast<float>(data.swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = data. swapChain->getExtent();
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    // Binder le descriptor set
    VkDescriptorSet descriptorSet = data.descriptorPool->getDescriptorSet(data.frameIndex);
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                          data.pipeline->getLayout(), 0, 1, &descriptorSet, 0, nullptr);

    // Dessiner les objets assignés à ce thread
    const auto& objects = data.scene->getObjects();
    uint32_t objectEnd = std::min(data.objectStart + data.objectCount, static_cast<uint32_t>(objects. size()));

    for (uint32_t i = data.objectStart; i < objectEnd; i++) {
        objects[i]->draw(cmdBuffer, data. pipeline->getLayout());
    }

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record secondary command buffer!");
    }
}