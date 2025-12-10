#include "DescriptorSetLayout.hpp"
#include "../../core/Device.hpp"
#include <vulkan/vulkan.h>
#include <array>
#include <stdexcept>
#include <iostream>

DescriptorSetLayout::DescriptorSetLayout(Device *device)
    : m_device(device), m_descriptorSetLayout(VK_NULL_HANDLE)
{
    createDescriptorSetLayout();
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    if (m_descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_device->getLogicalDevice(), m_descriptorSetLayout, nullptr);
    }
}

void DescriptorSetLayout::createDescriptorSetLayout()
{
    // 5 bindings : UBO + 4 textures PBR
    std::array<VkDescriptorSetLayoutBinding, 5> bindings{};

    // Binding 0 : Uniform Buffer Object (MVP matrices)
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    // Binding 1 : Base Color (Albedo)
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    // Binding 2 : Normal Map
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = nullptr;

    // Binding 3 : Metallic/Roughness
    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[3].pImmutableSamplers = nullptr;

    // Binding 4 : Ambient Occlusion
    bindings[4].binding = 4;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[4].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device->getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    std::cout << "✓ Descriptor set layout created (UBO + 4 PBR Textures)" << std::endl;
}