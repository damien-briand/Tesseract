#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "../Buffer/UniformBuffer.hpp"
#include "../Texture/Texture.hpp"
#include "../../core/Device.hpp"
#include <stdexcept>
#include <iostream>
#include <array>

DescriptorPool::DescriptorPool(Device* device, DescriptorSetLayout* layout,
                               UniformBuffer* uniformBuffer, Texture* texture,
                               uint32_t framesInFlight)
    : m_device(device)
    , m_texture(texture)
    , m_descriptorPool(VK_NULL_HANDLE)
{
    createDescriptorPool(framesInFlight);
    createDescriptorSets(layout, uniformBuffer, texture, framesInFlight);
}

DescriptorPool::~DescriptorPool() {
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_descriptorPool, nullptr);
    }
}

// Création du pool avec 2 types de descriptors
void DescriptorPool::createDescriptorPool(uint32_t framesInFlight) {
    // On a maintenant DEUX types de descriptors : UBO + Texture Sampler
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    
    // Pool size pour les Uniform Buffers (binding 0)
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = framesInFlight;
    
    // Pool size pour les Combined Image Samplers (binding 1)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = framesInFlight;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo. sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = framesInFlight;

    if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    std::cout << "✓ Descriptor pool created (UBO + Texture Sampler)" << std::endl;
}

// Configuration des descriptor sets avec UBO + Texture
void DescriptorPool::createDescriptorSets(DescriptorSetLayout* layout, UniformBuffer* uniformBuffer,
                                          Texture* texture, uint32_t framesInFlight) {
    // Allouer les descriptor sets (un par frame in flight)
    std::vector<VkDescriptorSetLayout> layouts(framesInFlight, layout->getHandle());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = framesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesInFlight);
    if (vkAllocateDescriptorSets(m_device->getLogicalDevice(), &allocInfo, m_descriptorSets. data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // Configurer chaque descriptor set
    for (size_t i = 0; i < framesInFlight; i++) {
        // Informations sur l'Uniform Buffer (binding 0)
        VkDescriptorBufferInfo bufferInfo = uniformBuffer->getDescriptorInfo(i);
        
        // Informations sur la Texture (binding 1)
        VkDescriptorImageInfo imageInfo = texture->getDescriptorInfo();

        // On a maintenant DEUX writes : un pour l'UBO, un pour la texture
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        // ===== WRITE 0 : Uniform Buffer Object =====
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0]. dstBinding = 0;  // Binding 0 dans le shader
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0]. descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;  // Pointeur vers les infos du buffer

        // 🆕 ===== WRITE 1 : Texture Sampler =====
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1]. dstBinding = 1;  // Binding 1 dans le shader
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1]. descriptorCount = 1;
        descriptorWrites[1]. pImageInfo = &imageInfo;  // Pointeur vers les infos de l'image

        // Mettre à jour les descriptor sets avec les DEUX writes
        vkUpdateDescriptorSets(
            m_device->getLogicalDevice(), 
            static_cast<uint32_t>(descriptorWrites.size()),  // 2 writes
            descriptorWrites.data(), 
            0, 
            nullptr
        );
    }

    std::cout << "✓ Descriptor sets created and configured (" << framesInFlight 
              << " sets with UBO + Texture)" << std::endl;
}