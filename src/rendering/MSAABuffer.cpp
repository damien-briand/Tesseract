#include "MSAABuffer.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include <stdexcept>
#include <iostream>

MSAABuffer::MSAABuffer(Device *device, SwapChain *swapChain, VkSampleCountFlagBits msaaSamples)
    : m_device(device), m_msaaSamples(msaaSamples), m_colorImage(VK_NULL_HANDLE), m_colorImageMemory(VK_NULL_HANDLE), m_colorImageView(VK_NULL_HANDLE)
{
    createColorResources(swapChain->getImageFormat(), swapChain->getExtent());

    std::cout << "✓ MSAA buffer created ("
              << swapChain->getExtent().width << "x" << swapChain->getExtent().height
              << ", " << msaaSamples << " samples)" << std::endl;
}

MSAABuffer::~MSAABuffer()
{
    if (m_colorImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device->getLogicalDevice(), m_colorImageView, nullptr);
    }
    if (m_colorImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_device->getLogicalDevice(), m_colorImage, nullptr);
    }
    if (m_colorImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device->getLogicalDevice(), m_colorImageMemory, nullptr);
    }
}

void MSAABuffer::createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent)
{
    // 1. Créer l'image multisamplée
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = swapChainImageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.samples = m_msaaSamples; // Le nombre de samples MSAA
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    if (vkCreateImage(m_device->getLogicalDevice(), &imageInfo, nullptr, &m_colorImage) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create MSAA color image!");
    }

    // 2. Allouer la mémoire
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->getLogicalDevice(), m_colorImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device->getLogicalDevice(), &allocInfo, nullptr, &m_colorImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate MSAA color image memory!");
    }

    vkBindImageMemory(m_device->getLogicalDevice(), m_colorImage, m_colorImageMemory, 0);

    // 3. Créer l'image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_colorImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device->getLogicalDevice(), &viewInfo, nullptr, &m_colorImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create MSAA color image view!");
    }
}

uint32_t MSAABuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type for MSAA buffer!");
}