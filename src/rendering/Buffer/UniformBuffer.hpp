#pragma once

#include "Buffer.hpp"
#include "UniformBufferObject.hpp"
#include <vector>

class Device;

/**
 * @brief Classe pour gérer les uniform buffers (un par frame in flight)
 */
class UniformBuffer {
public:
    UniformBuffer(Device* device, uint32_t framesInFlight);
    ~UniformBuffer();

    // Suppression de la copie
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    // Getters
    VkBuffer getBuffer(uint32_t frameIndex) const { return m_buffers[frameIndex]->getHandle(); }
    VkDescriptorBufferInfo getDescriptorInfo(uint32_t frameIndex) const;

    /**
     * @brief Met à jour les données du uniform buffer pour une frame
     * @param frameIndex Index de la frame
     * @param ubo Données à copier
     */
    void updateUniformBuffer(uint32_t frameIndex, const UniformBufferObject& ubo);

private:
    Device* m_device;
    std::vector<Buffer*> m_buffers;
    uint32_t m_framesInFlight;
};