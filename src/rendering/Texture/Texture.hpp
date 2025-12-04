#pragma once

#include <vulkan/vulkan.h>
#include <string>

class Device;

/**
 * @brief Classe pour gérer une texture Vulkan (image + view + sampler)
 */
class Texture
{
public:
    Texture(Device *device, const std::string &texturePath);
    ~Texture();

    // Suppression de la copie
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    // Getters
    VkImage getImage() const { return m_textureImage; }
    VkImageView getImageView() const { return m_textureImageView; }
    VkSampler getSampler() const { return m_textureSampler; }
    VkDescriptorImageInfo getDescriptorInfo() const;

private:
    Device *m_device;

    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_mipLevels;

    // Méthodes de création
    void createTextureImage(const std::string &texturePath);
    void createTextureImageView();
    void createTextureSampler();

    // Helpers
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    bool checkFormatSupport(VkFormat format);
};