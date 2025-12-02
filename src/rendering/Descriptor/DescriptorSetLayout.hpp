#pragma once

#include <vulkan/vulkan.h>

class Device;

/**
 * @brief Gère le layout des descriptor sets (binding uniform buffers)
 */
class DescriptorSetLayout {
public:
    DescriptorSetLayout(Device* device);
    ~DescriptorSetLayout();

    // Suppression de la copie
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    // Getters
    VkDescriptorSetLayout getHandle() const { return m_descriptorSetLayout; }

private:
    Device* m_device;
    VkDescriptorSetLayout m_descriptorSetLayout;

    void createDescriptorSetLayout();
};