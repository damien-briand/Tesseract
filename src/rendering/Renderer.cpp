#include "Renderer.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "CommandBuffer.hpp"
#include "MSAABuffer.hpp"
#include "Buffer/UniformBuffer.hpp"
#include "Buffer/UniformBufferObject.hpp"
#include "Descriptor/DescriptorPool.hpp"
#include "DepthBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/Window.hpp"
#include "../core/Camera.hpp"
#include "../scene/Scene.hpp"
#include <stdexcept>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(Device *device, SwapChain *swapChain, Window *window,
                   RenderPass *renderPass, Pipeline *pipeline,
                   CommandBuffer *commandBuffer, UniformBuffer *uniformBuffer,
                   DescriptorPool *descriptorPool, MSAABuffer *msaaBuffer, 
                   DepthBuffer *depthBuffer, uint32_t maxFramesInFlight,
                   Camera *camera, Scene *scene)
    : m_device(device), m_swapChain(swapChain), m_window(window),
      m_renderPass(renderPass), m_pipeline(pipeline),
      m_commandBuffer(commandBuffer), m_uniformBuffer(uniformBuffer),
      m_descriptorPool(descriptorPool), m_msaaBuffer(msaaBuffer), 
      m_depthBuffer(depthBuffer), m_maxFramesInFlight(maxFramesInFlight), 
      m_currentFrame(0), m_startTime(std::chrono::high_resolution_clock::now()), 
      m_camera(camera), m_scene(scene)  // AJOUTER scene
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

    updateUniformBuffer(m_currentFrame);

    m_commandBuffer->reset(m_currentFrame);
    
    // NOUVELLE SIGNATURE (avec Scene au lieu de VertexBuffer/IndexBuffer)
    m_commandBuffer->record(m_currentFrame, imageIndex, m_renderPass, m_pipeline, 
                           m_swapChain, m_descriptorPool, m_scene);

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
    presentInfo. sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->wasResized())
    {
        int width, height;
        m_window->getFramebufferSize(&width, &height);
        m_camera->updateAspectRatio(static_cast<float>(width),
                                    static_cast<float>(height));
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
            throw std:: runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void Renderer::recreateSwapChain()
{
    m_device->waitIdle();
    m_swapChain->recreate(m_renderPass->getHandle(),
                          m_msaaBuffer->getImageView(),
                          m_depthBuffer->getImageView());
}

void Renderer::updateUniformBuffer(uint32_t currentFrame)
{
    UniformBufferObject ubo{};

    // Model matrix (identité, chaque objet gère sa propre transformation)
    ubo.model = glm::mat4(1.0f);
    
    // View et Projection depuis la caméra
    ubo.view = m_camera->getViewMatrix();
    ubo.proj = m_camera->getProjectionMatrix();
    ubo.camPos = m_camera->getPosition();

    // Copier les données dans le uniform buffer
    m_uniformBuffer->updateUniformBuffer(currentFrame, ubo);
}