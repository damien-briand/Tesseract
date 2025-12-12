#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Device;
class DescriptorSetLayout;
class UniformBuffer;
class Material;
class LightManager;

class DescriptorPool
{
public:
    DescriptorPool(Device *device, DescriptorSetLayout *descriptorSetLayout,
                   UniformBuffer *uniformBuffer, Material *material,
                   LightManager *lightManager,
                   uint32_t maxFramesInFlight);
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool &operator=(const DescriptorPool &) = delete;

    VkDescriptorSet getDescriptorSet(uint32_t index) const;

private:
    Device *m_device;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    void createDescriptorPool(uint32_t maxFramesInFlight);
    void createDescriptorSets(DescriptorSetLayout *descriptorSetLayout,
                              UniformBuffer *uniformBuffer,
                              Material *material,
                              LightManager *lightManager,
                              uint32_t maxFramesInFlight);
};