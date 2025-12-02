#pragma once

#include <vulkan/vulkan.h>

class Device;

/**
 * @brief Classe de base pour gérer les buffers Vulkan (vertex, index, uniform, etc.)
 */
class Buffer {
public:
    Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, 
           VkMemoryPropertyFlags properties);
    ~Buffer();

    // Suppression de la copie
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Getters
    VkBuffer getHandle() const { return m_buffer; }
    VkDeviceMemory getMemory() const { return m_bufferMemory; }
    VkDeviceSize getSize() const { return m_size; }

    /**
     * @brief Mappe la mémoire du buffer pour écrire dedans
     * @return Pointeur vers la mémoire mappée
     */
    void* map();

    /**
     * @brief Unmap la mémoire du buffer
     */
    void unmap();

    /**
     * @brief Copie des données dans le buffer (map + memcpy + unmap)
     * @param data Pointeur vers les données
     * @param size Taille des données à copier
     */
    void copyData(const void* data, VkDeviceSize size);

    /**
     * @brief Copie le contenu d'un buffer vers un autre buffer
     * @param srcBuffer Buffer source
     * @param dstBuffer Buffer destination
     * @param size Taille à copier
     * @param device Device pour créer le command pool temporaire
     */
    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, Device* device);

private:
    Device* m_device;
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    VkDeviceSize m_size;

    void* m_mappedMemory;

    /**
     * @brief Trouve le bon type de mémoire selon les propriétés demandées
     * @param typeFilter Filtre des types de mémoire supportés
     * @param properties Propriétés désirées
     * @return Index du type de mémoire trouvé
     */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};