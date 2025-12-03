#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "Buffer/VertexBuffer.hpp"
#include "Buffer/IndexBuffer.hpp"
#include "Buffer/UniformBuffer.hpp"
#include "Buffer/UniformBufferObject.hpp"
#include "Descriptor/DescriptorPool.hpp"
#include "DepthBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/Window.hpp"
#include <stdexcept>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(Device *device, SwapChain *swapChain, Window *window,
                   RenderPass *renderPass, Pipeline *pipeline,
                   CommandBuffer *commandBuffer, VertexBuffer *vertexBuffer,
                   IndexBuffer *indexBuffer, UniformBuffer *uniformBuffer,
                   DescriptorPool *descriptorPool, DepthBuffer *depthBuffer,
                   uint32_t maxFramesInFlight)
    : m_device(device), m_swapChain(swapChain), m_window(window),
      m_renderPass(renderPass), m_pipeline(pipeline),
      m_commandBuffer(commandBuffer), m_vertexBuffer(vertexBuffer),
      m_indexBuffer(indexBuffer), m_uniformBuffer(uniformBuffer),
      m_descriptorPool(descriptorPool), m_depthBuffer(depthBuffer),
      m_maxFramesInFlight(maxFramesInFlight), m_currentFrame(0),
      m_startTime(std::chrono::high_resolution_clock::now())
{
    createSyncObjects();
}

Renderer::~Renderer()
{
    for (size_t i = 0; i < m_maxFramesInFlight; i++)
    {
        vkDestroySemaphore(m_device->getLogicalDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device->getLogicalDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device->getLogicalDevice(), m_inFlightFences[i], nullptr);
    }
}

void Renderer::drawFrame()
{
    vkWaitForFences(m_device->getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device->getLogicalDevice(),
        m_swapChain->getHandle(),
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    vkResetFences(m_device->getLogicalDevice(), 1, &m_inFlightFences[m_currentFrame]);

    // NOUVEAU : Mettre à jour le uniform buffer
    updateUniformBuffer(m_currentFrame);

    m_commandBuffer->reset(m_currentFrame);
    m_commandBuffer->record(m_currentFrame, imageIndex, m_renderPass, m_pipeline, m_swapChain,
                            m_vertexBuffer, m_indexBuffer, m_descriptorPool);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    VkCommandBuffer cmdBuffer = m_commandBuffer->getBuffer(m_currentFrame);
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->wasResized())
    {
        m_window->resetResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

void Renderer::waitIdle()
{
    m_device->waitIdle();
}

void Renderer::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(m_maxFramesInFlight);
    m_renderFinishedSemaphores.resize(m_maxFramesInFlight);
    m_inFlightFences.resize(m_maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device->getLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device->getLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void Renderer::recreateSwapChain()
{
    m_device->waitIdle();

    // Recréer le depth buffer avec les nouvelles dimensions
    delete m_depthBuffer;
    m_depthBuffer = new DepthBuffer(m_device, m_swapChain);

    m_swapChain->recreate(m_renderPass->getHandle(), m_depthBuffer->getImageView());
}

void Renderer::updateUniformBuffer(uint32_t currentFrame)
{
    // Calculer le temps écoulé depuis le démarrage
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime).count();

    UniformBufferObject ubo{};

    // Matrice Model : rotation autour de l'axe Z
    // ubo.model = glm::mat4(1.0f); // Pas de rotation pour l'instant
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Matrice View : caméra face au carré
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 1.0f, -1.0f), // Position : devant le carré sur l'axe Z
        glm::vec3(0.0f, 0.0f, 0.0f),  // Regarder vers : l'origine (centre du carré)
        glm::vec3(0.0f, 1.0f, 0.0f)   // Axe "up" : Y positif vers le haut
    );

    // Matrice Projection : perspective avec FOV de 45°
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                m_swapChain->getExtent().width / (float)m_swapChain->getExtent().height,
                                0.01f, 100.0f);

    // GLM a été conçu pour OpenGL où Y est inversé
    ubo.proj[1][1] *= -1;

    // Copier les données dans le uniform buffer
    m_uniformBuffer->updateUniformBuffer(currentFrame, ubo);
}