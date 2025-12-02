#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <chrono>

class Device;
class SwapChain;
class RenderPass;
class Pipeline;
class CommandBuffer;
class Window;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class DescriptorPool;

class Renderer {
public:
    Renderer(Device* device, SwapChain* swapChain, Window* window, 
             RenderPass* renderPass, Pipeline* pipeline, 
             CommandBuffer* commandBuffer, VertexBuffer* vertexBuffer,
             IndexBuffer* indexBuffer, UniformBuffer* uniformBuffer,
             DescriptorPool* descriptorPool, uint32_t maxFramesInFlight);
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
    VertexBuffer* m_vertexBuffer;
    IndexBuffer* m_indexBuffer;
    UniformBuffer* m_uniformBuffer;
    DescriptorPool* m_descriptorPool;

    uint32_t m_maxFramesInFlight;
    uint32_t m_currentFrame;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;

    void createSyncObjects();
    void recreateSwapChain();
    void updateUniformBuffer(uint32_t currentFrame);
};