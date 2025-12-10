#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Device;
class DescriptorSetLayout;
class UniformBuffer;
class Material;

/**
 * @brief Gère le pool de descriptors et les descriptor sets
 */
class DescriptorPool {
public:
    DescriptorPool(Device* device, DescriptorSetLayout* layout, 
                   UniformBuffer* uniformBuffer, Material* material,
                   uint32_t framesInFlight);
    ~DescriptorPool();

    // Suppression de la copie
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    // Getters
    VkDescriptorSet getDescriptorSet(uint32_t frameIndex) const { return m_descriptorSets[frameIndex]; }

private:
    Device* m_device;
    Material* m_material;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    void createDescriptorPool(uint32_t framesInFlight);
    
    void createDescriptorSets(DescriptorSetLayout* layout, UniformBuffer* uniformBuffer,
                              Material* material, uint32_t framesInFlight);
};