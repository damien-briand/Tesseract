#include "DescriptorSetLayout.hpp"
#include "../../core/Device.hpp"
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
    // Définir le binding pour le UBO
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;                                    // binding = 0 dans le shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;                            // Un seul UBO
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;        // Utilisé dans le vertex shader
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_device->getLogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    std::cout << "✓ Descriptor set layout created" << std::endl;
}