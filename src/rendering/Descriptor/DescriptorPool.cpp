#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "../Buffer/UniformBuffer.hpp"
#include "../../core/Device.hpp"
#include <stdexcept>
#include <iostream>

DescriptorPool::DescriptorPool(Device* device, DescriptorSetLayout* layout,
                               UniformBuffer* uniformBuffer, uint32_t framesInFlight)
    : m_device(device)
    , m_descriptorPool(VK_NULL_HANDLE)
{
    createDescriptorPool(framesInFlight);
    createDescriptorSets(layout, uniformBuffer, framesInFlight);
}

DescriptorPool::~DescriptorPool() {
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_descriptorPool, nullptr);
    }
}

void DescriptorPool::createDescriptorPool(uint32_t framesInFlight) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = framesInFlight;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = framesInFlight;

    if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    std::cout << "✓ Descriptor pool created" << std::endl;
}

void DescriptorPool::createDescriptorSets(DescriptorSetLayout* layout, UniformBuffer* uniformBuffer,
                                          uint32_t framesInFlight) {
    std::vector<VkDescriptorSetLayout> layouts(framesInFlight, layout->getHandle());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = framesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesInFlight);
    if (vkAllocateDescriptorSets(m_device->getLogicalDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // Configurer chaque descriptor set pour pointer vers son uniform buffer
    for (size_t i = 0; i < framesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo = uniformBuffer->getDescriptorInfo(i);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device->getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    std::cout << "✓ Descriptor sets created and configured (" << framesInFlight << " sets)" << std::endl;
}