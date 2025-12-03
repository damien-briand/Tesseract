#include "RenderPass.hpp"
#include "../core/Device.hpp"
#include <stdexcept>
#include <array>
#include <iostream>

// Constructeur avec depthFormat
RenderPass::RenderPass(Device* device, VkFormat swapChainImageFormat, VkFormat depthFormat)
    : m_device(device)
    , m_renderPass(VK_NULL_HANDLE)
{
    createRenderPass(swapChainImageFormat, depthFormat);
}

RenderPass::~RenderPass() {
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device->getLogicalDevice(), m_renderPass, nullptr);
    }
}

// Fonction avec depthFormat
void RenderPass::createRenderPass(VkFormat swapChainImageFormat, VkFormat depthFormat) {
    // ===== ATTACHMENT 0 : Color Attachment (comme avant) =====
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // Clear au début
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;      // Sauvegarder à la fin
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // ===== ATTACHMENT 1 : Depth Attachment (NOUVEAU) =====
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // Clear à 1. 0 au début
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // Pas besoin de sauvegarder
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // ===== References aux attachments =====
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;  // Index 0 = color attachment
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Reference au depth attachment
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;  // Index 1 = depth attachment
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // ===== Subpass =====
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // ===== Dependency modifiée pour inclure le depth =====
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency. dstSubpass = 0;
    // Inclure le depth dans les stages
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // ===== Créer le render pass avec 2 attachments =====
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device->getLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }

    std::cout << "✓ Render pass created with depth attachment" << std::endl;
}