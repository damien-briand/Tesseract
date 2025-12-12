#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "../Buffer/UniformBuffer.hpp"
#include "../Material/Material.hpp"
#include "../Texture/Texture.hpp"
#include "../../core/Device.hpp"
#include "../../scene/LightManager.hpp"
#include <stdexcept>
#include <array>
#include <iostream>

DescriptorPool:: DescriptorPool(Device* device, DescriptorSetLayout* descriptorSetLayout,
                               UniformBuffer* uniformBuffer, Material* material,
                               LightManager* lightManager,
                               uint32_t maxFramesInFlight)
    : m_device(device)
    , m_descriptorPool(VK_NULL_HANDLE)
{
    createDescriptorPool(maxFramesInFlight);
    createDescriptorSets(descriptorSetLayout, uniformBuffer, material, lightManager, maxFramesInFlight);
}

DescriptorPool:: ~DescriptorPool() {
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_descriptorPool, nullptr);
    }
}

void DescriptorPool:: createDescriptorPool(uint32_t maxFramesInFlight) {
    // Pool sizes pour tous les types de descriptors
    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    
    // Uniform buffers
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = maxFramesInFlight;
    
    // Storage buffers (pour les lumières)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = maxFramesInFlight;
    
    // Combined image samplers (4 textures :  color, normal, metallic, ao)
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = maxFramesInFlight * 4;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo. sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxFramesInFlight;

    if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void DescriptorPool::createDescriptorSets(DescriptorSetLayout* descriptorSetLayout,
                                         UniformBuffer* uniformBuffer, 
                                         Material* material,
                                         LightManager* lightManager,
                                         uint32_t maxFramesInFlight) {
    std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout->getHandle());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = maxFramesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(maxFramesInFlight);
    if (vkAllocateDescriptorSets(m_device->getLogicalDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std:: runtime_error("Failed to allocate descriptor sets!");
    }

    // Mettre à jour chaque descriptor set
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        // Binding 0 :  Uniform Buffer
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer->getBuffer(i);
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // Binding 1 :   Storage Buffer (lights)
        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightManager->getBuffer();
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = VK_WHOLE_SIZE;

        // Binding 2 :  Base Color Texture
        VkDescriptorImageInfo colorImageInfo = material->getBaseColorInfo();

        // Binding 3 :  Normal Texture
        VkDescriptorImageInfo normalImageInfo = material->getNormalInfo();

        // Binding 4 : Metallic/Roughness Texture
        VkDescriptorImageInfo metallicImageInfo = material->getMetallicRoughnessInfo();

        // Binding 5 : AO Texture
        VkDescriptorImageInfo aoImageInfo = material->getAOInfo();

        // Créer les writes
        std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

        // Write 0 : Uniform Buffer
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // Write 1 : Storage Buffer (lights)
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1]. dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightBufferInfo;

        // Write 2 : Base Color Texture
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &colorImageInfo;

        // Write 3 : Normal Texture
        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = m_descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &normalImageInfo;

        // Write 4 : Metallic/Roughness Texture
        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = m_descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &metallicImageInfo;

        // Write 5 : AO Texture
        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = m_descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &aoImageInfo;

        vkUpdateDescriptorSets(m_device->getLogicalDevice(), 
                              static_cast<uint32_t>(descriptorWrites. size()), 
                              descriptorWrites.data(), 
                              0, nullptr);
    }

    std::cout << "✓ Descriptor sets created with light buffer support (6 bindings total)" << std::endl;
}

VkDescriptorSet DescriptorPool::getDescriptorSet(uint32_t index) const {
    return m_descriptorSets[index];
}