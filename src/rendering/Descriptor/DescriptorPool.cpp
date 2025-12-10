#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "../Buffer/UniformBuffer.hpp"
#include "../Texture/Texture.hpp"
#include "../../core/Device.hpp"
#include "../Material/Material.hpp"
#include <stdexcept>
#include <iostream>
#include <array>

DescriptorPool::DescriptorPool(Device *device, DescriptorSetLayout *layout,
                               UniformBuffer *uniformBuffer, Material* material,
                               uint32_t framesInFlight)
    : m_device(device), m_material(material), m_descriptorPool(VK_NULL_HANDLE)
{
    createDescriptorPool(framesInFlight);
    createDescriptorSets(layout, uniformBuffer, material, framesInFlight);
}

DescriptorPool::~DescriptorPool()
{
    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_descriptorPool, nullptr);
    }
}

// Création du pool avec 2 types de descriptors
void DescriptorPool::createDescriptorPool(uint32_t framesInFlight)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    // Pool pour les Uniform Buffers
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = framesInFlight;

    // Pool pour les Combined Image Samplers (4 textures par frame)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = framesInFlight * 4; // 4 textures PBR

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = framesInFlight;

    if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    std::cout << "✓ Descriptor pool created (UBO + 4 PBR Textures)" << std::endl;
}

// Configuration des descriptor sets avec UBO + Texture
void DescriptorPool::createDescriptorSets(DescriptorSetLayout *layout, UniformBuffer *uniformBuffer,
                                          Material *material, uint32_t framesInFlight)
{
    // Allouer les descriptor sets (un par frame in flight)
    std::vector<VkDescriptorSetLayout> layouts(framesInFlight, layout->getHandle());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = framesInFlight;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(framesInFlight);
    if (vkAllocateDescriptorSets(m_device->getLogicalDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // Configurer chaque descriptor set
    for (size_t i = 0; i < framesInFlight; i++)
    {
        VkDescriptorBufferInfo bufferInfo = uniformBuffer->getDescriptorInfo(i);
        
        // Infos pour les 4 textures
        VkDescriptorImageInfo baseColorInfo = material->getBaseColorInfo();
        VkDescriptorImageInfo normalInfo = material->getNormalInfo();
        VkDescriptorImageInfo metallicRoughnessInfo = material->getMetallicRoughnessInfo();
        VkDescriptorImageInfo aoInfo = material->getAOInfo();

        // 5 writes : UBO + 4 textures
        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};

        // Write 0 : UBO
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // Write 1 : Base Color
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &baseColorInfo;

        // Write 2 : Normal Map
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &normalInfo;

        // Write 3 : Metallic/Roughness
        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = m_descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &metallicRoughnessInfo;

        // Write 4 : AO
        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = m_descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &aoInfo;

        vkUpdateDescriptorSets(
            m_device->getLogicalDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr);
    }

    std::cout << "✓ Descriptor sets created and configured (" << framesInFlight
              << " sets with UBO + Texture)" << std::endl;
}