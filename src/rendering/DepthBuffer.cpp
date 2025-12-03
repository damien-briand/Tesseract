#include "DepthBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include <stdexcept>
#include <array>
#include <iostream>

DepthBuffer::DepthBuffer(Device* device, SwapChain* swapChain)
    : m_device(device)
    , m_depthImage(VK_NULL_HANDLE)
    , m_depthImageMemory(VK_NULL_HANDLE)
    , m_depthImageView(VK_NULL_HANDLE)
{
    createDepthResources(swapChain->getExtent());
    
    std::cout << "✓ Depth buffer created (" 
              << swapChain->getExtent().width << "x" << swapChain->getExtent().height 
              << ", format: " << m_depthFormat << ")" << std::endl;
}

DepthBuffer::~DepthBuffer() {
    if (m_depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->getLogicalDevice(), m_depthImageView, nullptr);
    }
    if (m_depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device->getLogicalDevice(), m_depthImage, nullptr);
    }
    if (m_depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->getLogicalDevice(), m_depthImageMemory, nullptr);
    }
}

void DepthBuffer::createDepthResources(VkExtent2D swapChainExtent) {
    // 1. Trouver le meilleur format de profondeur supporté
    m_depthFormat = findDepthFormat();

    // 2. Créer l'image de profondeur
    VkImageCreateInfo imageInfo{};
    imageInfo. sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;  // Optimal pour le GPU
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device->getLogicalDevice(), &imageInfo, nullptr, &m_depthImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image!");
    }

    // 3. Allouer la mémoire pour l'image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->getLogicalDevice(), m_depthImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device->getLogicalDevice(), &allocInfo, nullptr, &m_depthImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate depth image memory!");
    }

    vkBindImageMemory(m_device->getLogicalDevice(), m_depthImage, m_depthImageMemory, 0);

    // 4. Créer l'image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device->getLogicalDevice(), &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image view!");
    }
}

VkFormat DepthBuffer::findDepthFormat() {
    // Chercher le meilleur format de profondeur parmi ces candidats
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat DepthBuffer::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                         VkImageTiling tiling,
                                         VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_device->getPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported depth format!");
}

bool DepthBuffer::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t DepthBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type for depth buffer!");
}