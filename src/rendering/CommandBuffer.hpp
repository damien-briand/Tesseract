#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Device;
class RenderPass;
class Pipeline;
class SwapChain;
class DescriptorPool;
class Scene;

class CommandBuffer {
public:
    CommandBuffer(Device* device, uint32_t maxFramesInFlight);
    ~CommandBuffer();

    // Suppression de la copie
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    // Getters
    VkCommandPool getPool() const { return m_commandPool; }
    VkCommandBuffer getBuffer(uint32_t index) const { return m_commandBuffers[index]; }
    const std::vector<VkCommandBuffer>& getBuffers() const { return m_commandBuffers; }

    // Méthodes - SIGNATURE MODIFIÉE
    void reset(uint32_t index);
    void record(uint32_t index, uint32_t imageIndex, RenderPass* renderPass, 
                Pipeline* pipeline, SwapChain* swapChain,
                DescriptorPool* descriptorPool, Scene* scene);  // CHANGÉ

private:
    Device* m_device;
    VkCommandPool m_commandPool;
    std:: vector<VkCommandBuffer> m_commandBuffers;

    void createCommandPool();
    void createCommandBuffers(uint32_t count);
};