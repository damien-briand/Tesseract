#pragma once

#include <vulkan/vulkan.h>

class Device;
class SwapChain;

/**
 * @brief Gère le buffer MSAA (Multi-Sample Anti-Aliasing)
 */
class MSAABuffer
{
public:
    MSAABuffer(Device *device, SwapChain *swapChain, VkSampleCountFlagBits msaaSamples);
    ~MSAABuffer();

    // Suppression de la copie
    MSAABuffer(const MSAABuffer &) = delete;
    MSAABuffer &operator=(const MSAABuffer &) = delete;

    // Getters
    VkImage getImage() const { return m_colorImage; }
    VkImageView getImageView() const { return m_colorImageView; }
    VkSampleCountFlagBits getSampleCount() const { return m_msaaSamples; }

private:
    Device *m_device;
    VkSampleCountFlagBits m_msaaSamples;

    VkImage m_colorImage;
    VkDeviceMemory m_colorImageMemory;
    VkImageView m_colorImageView;

    void createColorResources(VkFormat swapChainImageFormat, VkExtent2D swapChainExtent);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};