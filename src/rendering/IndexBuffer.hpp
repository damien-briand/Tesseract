#pragma once

#include "Buffer.hpp"
#include <vector>
#include <cstdint>

class Device;

/**
 * @brief Classe spécialisée pour gérer un index buffer
 */
class IndexBuffer {
public:
    IndexBuffer(Device* device, const std::vector<uint16_t>& indices);
    ~IndexBuffer();

    // Suppression de la copie
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // Getters
    VkBuffer getHandle() const { return m_buffer->getHandle(); }
    uint32_t getIndexCount() const { return m_indexCount; }
    VkIndexType getIndexType() const { return VK_INDEX_TYPE_UINT16; }

    /**
     * @brief Bind l'index buffer à un command buffer
     * @param commandBuffer Command buffer où binder
     */
    void bind(VkCommandBuffer commandBuffer);

private:
    Device* m_device;
    Buffer* m_buffer;
    uint32_t m_indexCount;
};