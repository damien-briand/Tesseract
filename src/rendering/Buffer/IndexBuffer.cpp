#include "IndexBuffer.hpp"
#include "../../core/Device.hpp"
#include <iostream>

IndexBuffer::~IndexBuffer() {
    delete m_buffer;
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, m_buffer->getHandle(), 0, m_indexType);  // Utilise m_indexType dynamique
}