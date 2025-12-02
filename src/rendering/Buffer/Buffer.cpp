#include "Buffer.hpp"
#include "../../core/Device.hpp"
#include <stdexcept>
#include <cstring>

Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : m_device(device)
    , m_buffer(VK_NULL_HANDLE)
    , m_bufferMemory(VK_NULL_HANDLE)
    , m_size(size)
    , m_mappedMemory(nullptr)
{
    // 1. Créer le buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device->getLogicalDevice(), &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    // 2. Récupérer les requirements de mémoire
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getLogicalDevice(), m_buffer, &memRequirements);

    // 3. Allouer la mémoire
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device->getLogicalDevice(), &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    // 4. Lier le buffer à la mémoire
    vkBindBufferMemory(m_device->getLogicalDevice(), m_buffer, m_bufferMemory, 0);
}

Buffer::~Buffer() {
    if (m_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->getLogicalDevice(), m_buffer, nullptr);
    }
    if (m_bufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->getLogicalDevice(), m_bufferMemory, nullptr);
    }
}

void* Buffer::map() {
    if (m_mappedMemory == nullptr) {
        vkMapMemory(m_device->getLogicalDevice(), m_bufferMemory, 0, m_size, 0, &m_mappedMemory);
    }
    return m_mappedMemory;
}

void Buffer::unmap() {
    if (m_mappedMemory != nullptr) {
        vkUnmapMemory(m_device->getLogicalDevice(), m_bufferMemory);
        m_mappedMemory = nullptr;
    }
}

void Buffer::copyData(const void* data, VkDeviceSize size) {
    void* mappedData = map();
    memcpy(mappedData, data, static_cast<size_t>(size));
    unmap();
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, Device* device) {
    // Créer un command pool temporaire pour la copie
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; // Optimisé pour des commandes courtes
    poolInfo.queueFamilyIndex = device->getQueueFamilies().graphicsFamily.value();

    VkCommandPool commandPool;
    vkCreateCommandPool(device->getLogicalDevice(), &poolInfo, nullptr, &commandPool);

    // Allouer un command buffer temporaire
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, &commandBuffer);

    // Enregistrer la commande de copie
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    // Soumettre et attendre
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());

    // Cleanup
    vkFreeCommandBuffers(device->getLogicalDevice(), commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(device->getLogicalDevice(), commandPool, nullptr);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_device->getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}