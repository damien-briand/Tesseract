#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Window;
class Device;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
public:
    SwapChain(Device *device, VkSurfaceKHR surface, Window *window);
    ~SwapChain();

    // Suppression de la copie
    SwapChain(const SwapChain &) = delete;
    SwapChain &operator=(const SwapChain &) = delete;

    // Getters
    VkSwapchainKHR getHandle() const { return m_swapChain; }
    VkFormat getImageFormat() const { return m_imageFormat; }
    VkExtent2D getExtent() const { return m_extent; }
    const std::vector<VkImage> &getImages() const { return m_images; }
    const std::vector<VkImageView> &getImageViews() const { return m_imageViews; }
    const std::vector<VkFramebuffer> &getFramebuffers() const { return m_framebuffers; }
    size_t getImageCount() const { return m_images.size(); }

    // Méthodes
    void createFramebuffers(VkRenderPass renderPass, VkImageView msaaImageView, VkImageView depthImageView);
    void cleanup();
    void recreate(VkRenderPass renderPass, VkImageView msaaImageView, VkImageView depthImageView);

private:
    Device *m_device;
    VkSurfaceKHR m_surface;
    Window *m_window;

    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkFormat m_imageFormat;
    VkExtent2D m_extent;

    // Méthodes privées
    void createSwapChain();
    void createImageViews();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};