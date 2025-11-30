#pragma once

#include <vulkan/vulkan.h>

class Device;

class RenderPass {
public:
    RenderPass(Device* device, VkFormat swapChainImageFormat);
    ~RenderPass();

    // Suppression de la copie
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    // Getters
    VkRenderPass getHandle() const { return m_renderPass; }

private:
    Device* m_device;
    VkRenderPass m_renderPass;

    void createRenderPass(VkFormat swapChainImageFormat);
};