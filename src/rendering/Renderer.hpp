#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class Device;
class SwapChain;
class RenderPass;
class Pipeline;
class CommandBuffer;
class Window;

class Renderer {
public:
    Renderer(Device* device, SwapChain* swapChain, Window* window, 
             RenderPass* renderPass, Pipeline* pipeline, 
             CommandBuffer* commandBuffer, uint32_t maxFramesInFlight);
    ~Renderer();

    // Suppression de la copie
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Méthodes
    void drawFrame();
    void waitIdle();

private:
    Device* m_device;
    SwapChain* m_swapChain;
    Window* m_window;
    RenderPass* m_renderPass;
    Pipeline* m_pipeline;
    CommandBuffer* m_commandBuffer;

    uint32_t m_maxFramesInFlight;
    uint32_t m_currentFrame;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    void createSyncObjects();
    void recreateSwapChain();
};