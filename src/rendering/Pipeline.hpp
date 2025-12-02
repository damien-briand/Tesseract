#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Device;
class RenderPass;
class DescriptorSetLayout;

class Pipeline {
public:
    Pipeline(Device* device, RenderPass* renderPass, DescriptorSetLayout* descriptorSetLayout,
             VkExtent2D swapChainExtent, const std::string& vertShaderPath, const std::string& fragShaderPath);
    ~Pipeline();

    // Suppression de la copie
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // Getters
    VkPipeline getHandle() const { return m_graphicsPipeline; }
    VkPipelineLayout getLayout() const { return m_pipelineLayout; }

private:
    Device* m_device;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    void createGraphicsPipeline(RenderPass* renderPass, DescriptorSetLayout* descriptorSetLayout,
                                VkExtent2D swapChainExtent, const std::string& vertShaderPath, 
                                const std::string& fragShaderPath);
};