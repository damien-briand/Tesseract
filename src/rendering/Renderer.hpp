#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <chrono>

class Device;
class SwapChain;
class RenderPass;
class Pipeline;
class AdvancedCommandRecorder; // CHANGÉ
class Window;
class UniformBuffer;
class DescriptorPool;
class DepthBuffer;
class MSAABuffer;
class Camera;
class Scene;

class Renderer
{
public:
    Renderer(Device *device, SwapChain *swapChain, Window *window,
             RenderPass *renderPass, Pipeline *pipeline,
             AdvancedCommandRecorder *commandRecorder, // CHANGÉ
             UniformBuffer *uniformBuffer,
             DescriptorPool *descriptorPool, MSAABuffer *msaaBuffer,
             DepthBuffer *depthBuffer, uint32_t maxFramesInFlight,
             Camera *camera, Scene *scene);
    ~Renderer();

    // Suppression de la copie
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    // Méthodes
    void drawFrame();
    void waitIdle();

private:
    Device *m_device;
    SwapChain *m_swapChain;
    Window *m_window;
    RenderPass *m_renderPass;
    Pipeline *m_pipeline;
    AdvancedCommandRecorder *m_commandRecorder; // CHANGÉ
    UniformBuffer *m_uniformBuffer;
    DescriptorPool *m_descriptorPool;
    DepthBuffer *m_depthBuffer;
    MSAABuffer *m_msaaBuffer;
    Camera *m_camera;
    Scene *m_scene;

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