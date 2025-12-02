#include "CommandBuffer.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "Buffer/VertexBuffer.hpp"
#include "Buffer/IndexBuffer.hpp"
#include "Descriptor/DescriptorPool.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include <stdexcept>

CommandBuffer::CommandBuffer(Device *device, uint32_t maxFramesInFlight)
    : m_device(device), m_commandPool(VK_NULL_HANDLE)
{
    createCommandPool();
    createCommandBuffers(maxFramesInFlight);
}

CommandBuffer::~CommandBuffer()
{
    if (m_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device->getLogicalDevice(), m_commandPool, nullptr);
    }
}

void CommandBuffer::reset(uint32_t index)
{
    vkResetCommandBuffer(m_commandBuffers[index], 0);
}

void CommandBuffer::record(uint32_t index, uint32_t imageIndex, RenderPass *renderPass,
                           Pipeline *pipeline, SwapChain *swapChain, VertexBuffer *vertexBuffer,
                           IndexBuffer *indexBuffer, DescriptorPool *descriptorPool)
{
    VkCommandBuffer commandBuffer = m_commandBuffers[index];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->getHandle();
    renderPassInfo.framebuffer = swapChain->getFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getExtent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getHandle());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getExtent().width);
    viewport.height = static_cast<float>(swapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Binder le vertex buffer
    vertexBuffer->bind(commandBuffer);

    // Binder l'index buffer
    indexBuffer->bind(commandBuffer);

    // NOUVEAU : Binder le descriptor set
    VkDescriptorSet descriptorSet = descriptorPool->getDescriptorSet(index);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &descriptorSet, 0, nullptr);

    // Draw
    vkCmdDrawIndexed(commandBuffer, indexBuffer->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void CommandBuffer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = m_device->getQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device->getLogicalDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void CommandBuffer::createCommandBuffers(uint32_t count)
{
    m_commandBuffers.resize(count);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;

    if (vkAllocateCommandBuffers(m_device->getLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}