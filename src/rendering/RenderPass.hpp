#pragma once

#include <vulkan/vulkan.h>

class Device;

class RenderPass {
public:
    // Ajout du paramètre depthFormat
    RenderPass(Device* device, VkFormat swapChainImageFormat, VkFormat depthFormat);
    ~RenderPass();

    // Suppression de la copie
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    // Getters
    VkRenderPass getHandle() const { return m_renderPass; }

private:
    Device* m_device;
    VkRenderPass m_renderPass;

    // Ajout du paramètre depthFormat
    void createRenderPass(VkFormat swapChainImageFormat, VkFormat depthFormat);
};