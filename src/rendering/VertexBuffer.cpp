#include "VertexBuffer.hpp"
#include "../core/Device.hpp"
#include <iostream>

VertexBuffer::VertexBuffer(Device* device, const std::vector<Vertex>& vertices)
    : m_device(device)
    , m_vertexCount(static_cast<uint32_t>(vertices.size()))
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    // 1. Créer le STAGING BUFFER (accessible par le CPU)
    Buffer stagingBuffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,  // Source de transfert
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    // 2. Copier les données dans le staging buffer
    stagingBuffer.copyData(vertices.data(), bufferSize);

    // 3. Créer le VERTEX BUFFER final (mémoire GPU optimisée)
    m_buffer = new Buffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,  // Destination + vertex buffer
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  // Mémoire GPU rapide !
    );

    // 4. Copier du staging buffer vers le vertex buffer GPU
    Buffer::copyBuffer(stagingBuffer.getHandle(), m_buffer->getHandle(), bufferSize, m_device);

    // 5. Le staging buffer est automatiquement détruit ici (fin de scope)

    std::cout << "✓ Vertex buffer created with staging (" << m_vertexCount << " vertices, " 
              << bufferSize << " bytes, DEVICE_LOCAL)" << std::endl;
}

VertexBuffer::~VertexBuffer() {
    delete m_buffer;
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {m_buffer->getHandle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}