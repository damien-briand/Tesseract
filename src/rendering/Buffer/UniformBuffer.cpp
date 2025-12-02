#include "UniformBuffer.hpp"
#include "../../core/Device.hpp"
#include <iostream>

UniformBuffer::UniformBuffer(Device* device, uint32_t framesInFlight)
    : m_device(device)
    , m_framesInFlight(framesInFlight)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_buffers.resize(framesInFlight);

    // Créer un uniform buffer par frame in flight
    for (size_t i = 0; i < framesInFlight; i++) {
        m_buffers[i] = new Buffer(
            m_device,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
    }

    std::cout << "✓ Uniform buffers created (" << framesInFlight << " buffers, " 
              << bufferSize << " bytes each)" << std::endl;
}

UniformBuffer::~UniformBuffer() {
    for (auto buffer : m_buffers) {
        delete buffer;
    }
}

VkDescriptorBufferInfo UniformBuffer::getDescriptorInfo(uint32_t frameIndex) const {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_buffers[frameIndex]->getHandle();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);
    return bufferInfo;
}

void UniformBuffer::updateUniformBuffer(uint32_t frameIndex, const UniformBufferObject& ubo) {
    m_buffers[frameIndex]->copyData(&ubo, sizeof(UniformBufferObject));
}