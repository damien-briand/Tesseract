#include "IndexBuffer.hpp"
#include "../core/Device.hpp"
#include <iostream>

IndexBuffer::IndexBuffer(Device* device, const std::vector<uint16_t>& indices)
    : m_device(device)
    , m_indexCount(static_cast<uint32_t>(indices.size()))
{
    VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();

    // 1. Créer le STAGING BUFFER (accessible par le CPU)
    Buffer stagingBuffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    // 2. Copier les indices dans le staging buffer
    stagingBuffer.copyData(indices.data(), bufferSize);

    // 3. Créer l'INDEX BUFFER final (mémoire GPU optimisée)
    m_buffer = new Buffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    // 4. Copier du staging buffer vers l'index buffer GPU
    Buffer::copyBuffer(stagingBuffer.getHandle(), m_buffer->getHandle(), bufferSize, m_device);

    std::cout << "✓ Index buffer created with staging (" << m_indexCount << " indices, " 
              << bufferSize << " bytes, DEVICE_LOCAL)" << std::endl;
}

IndexBuffer::~IndexBuffer() {
    delete m_buffer;
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, m_buffer->getHandle(), 0, VK_INDEX_TYPE_UINT16);
}