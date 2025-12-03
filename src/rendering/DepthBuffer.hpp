#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Device;
class SwapChain;

/**
 * @brief Gère le depth buffer (Z-buffer) pour le depth testing
 */
class DepthBuffer {
public:
    DepthBuffer(Device* device, SwapChain* swapChain);
    ~DepthBuffer();

    // Suppression de la copie
    DepthBuffer(const DepthBuffer&) = delete;
    DepthBuffer& operator=(const DepthBuffer&) = delete;

    // Getters
    VkImage getImage() const { return m_depthImage; }
    VkImageView getImageView() const { return m_depthImageView; }
    VkFormat getFormat() const { return m_depthFormat; }

private:
    Device* m_device;
    
    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;
    VkFormat m_depthFormat;

    // Méthodes de création
    void createDepthResources(VkExtent2D swapChainExtent);
    
    // Helpers
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, 
                                 VkImageTiling tiling, 
                                 VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};