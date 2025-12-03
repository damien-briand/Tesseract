#include "DescriptorSetLayout.hpp"
#include "../../core/Device.hpp"
#include <vulkan/vulkan.h>
#include <array>
#include <stdexcept>
#include <iostream>

DescriptorSetLayout::DescriptorSetLayout(Device* device)
    : m_device(device)
    , m_descriptorSetLayout(VK_NULL_HANDLE)
{
    createDescriptorSetLayout();
}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (m_descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device->getLogicalDevice(), m_descriptorSetLayout, nullptr);
    }
}

void DescriptorSetLayout::createDescriptorSetLayout() {
    // Maintenant on a DEUX bindings : UBO (0) + Sampler (1)
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    
    // Binding 0 : Uniform Buffer Object (MVP matrices)
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  // Utilisé dans le vertex shader
    bindings[0].pImmutableSamplers = nullptr;

    // Binding 1 : Combined Image Sampler (Texture)
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // Utilisé dans le fragment shader
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());  // 2 bindings
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device->getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    std::cout << "✓ Descriptor set layout created (UBO + Texture Sampler)" << std::endl;
}