#pragma once

#include "Buffer.hpp"
#include <iostream>
#include <vector>
#include <cstdint>

class Device;

/**
 * @brief Classe spécialisée pour gérer un index buffer
 * Support uint16_t et uint32_t
 */
class IndexBuffer
{
public:
    //  Constructeur template pour supporter uint16_t et uint32_t
    template <typename T>
    IndexBuffer(Device *device, const std::vector<T> &indices);

    ~IndexBuffer();

    // Suppression de la copie
    IndexBuffer(const IndexBuffer &) = delete;
    IndexBuffer &operator=(const IndexBuffer &) = delete;

    // Getters
    VkBuffer getHandle() const { return m_buffer->getHandle(); }
    uint32_t getIndexCount() const { return m_indexCount; }
    VkIndexType getIndexType() const { return m_indexType; } //  Dynamique

    /**
     * @brief Bind l'index buffer à un command buffer
     * @param commandBuffer Command buffer où binder
     */
    void bind(VkCommandBuffer commandBuffer);

private:
    Device *m_device;
    Buffer *m_buffer;
    uint32_t m_indexCount;
    VkIndexType m_indexType; // Type d'index (UINT16 ou UINT32)

    // Fonction privée pour créer le buffer
    template <typename T>
    void createBuffer(const std::vector<T> &indices);
};

//  Implémentation du template dans le header
template <typename T>
IndexBuffer::IndexBuffer(Device *device, const std::vector<T> &indices)
    : m_device(device), m_indexCount(static_cast<uint32_t>(indices.size()))
{
    // Déterminer le type d'index
    if constexpr (sizeof(T) == sizeof(uint16_t))
    {
        m_indexType = VK_INDEX_TYPE_UINT16;
    }
    else if constexpr (sizeof(T) == sizeof(uint32_t))
    {
        m_indexType = VK_INDEX_TYPE_UINT32;
    }
    else
    {
        static_assert(sizeof(T) == sizeof(uint16_t) || sizeof(T) == sizeof(uint32_t),
                      "Index type must be uint16_t or uint32_t");
    }

    createBuffer(indices);
}

template <typename T>
void IndexBuffer::createBuffer(const std::vector<T> &indices)
{
    VkDeviceSize bufferSize = sizeof(T) * indices.size();

    // 1.  Créer le STAGING BUFFER (accessible par le CPU)
    Buffer stagingBuffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 2. Copier les indices dans le staging buffer
    stagingBuffer.copyData(indices.data(), bufferSize);

    // 3. Créer l'INDEX BUFFER final (mémoire GPU optimisée)
    m_buffer = new Buffer(
        m_device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 4.  Copier du staging buffer vers l'index buffer GPU
    Buffer::copyBuffer(stagingBuffer.getHandle(), m_buffer->getHandle(), bufferSize, m_device);

    const char *indexTypeName = (m_indexType == VK_INDEX_TYPE_UINT16) ? "UINT16" : "UINT32";
    std::cout << "✓ Index buffer created (" << m_indexCount << " indices, "
              << bufferSize << " bytes, " << indexTypeName << ")" << std::endl;
}