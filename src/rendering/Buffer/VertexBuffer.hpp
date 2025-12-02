#pragma once

#include "Buffer.hpp"
#include "../Vertex.hpp"
#include <vector>

class Device;

/**
 * @brief Classe spécialisée pour gérer un vertex buffer
 */
class VertexBuffer {
public:
    VertexBuffer(Device* device, const std::vector<Vertex>& vertices);
    ~VertexBuffer();

    // Suppression de la copie
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Getters
    VkBuffer getHandle() const { return m_buffer->getHandle(); }
    uint32_t getVertexCount() const { return m_vertexCount; }

    /**
     * @brief Bind le vertex buffer à un command buffer
     * @param commandBuffer Command buffer où binder
     */
    void bind(VkCommandBuffer commandBuffer);

private:
    Device* m_device;
    Buffer* m_buffer;
    uint32_t m_vertexCount;
};