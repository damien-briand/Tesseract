#include "LightManager.hpp"
#include "../core/Device.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>

// Taille maximale du buffer (on alloue pour un nombre max de lumières)
static constexpr uint32_t MAX_LIGHTS = 16;

LightManager::LightManager(Device *device)
    : m_device(device), m_ambientColor(0.03f, 0.03f, 0.03f) // Ambient très faible par défaut
      ,
      m_ambientIntensity(1.0f), m_lightBuffer(VK_NULL_HANDLE), m_lightBufferMemory(VK_NULL_HANDLE), m_lightBufferMapped(nullptr), m_bufferSize(0)
{
    createLightBuffer();
    std::cout << "✓ Light Manager created (max " << MAX_LIGHTS << " lights)" << std::endl;
}

LightManager::~LightManager()
{
    destroyLightBuffer();
    m_lights.clear();
}

void LightManager::addLight(Light *light)
{
    if (m_lights.size() >= MAX_LIGHTS)
    {
        std::cerr << "✗ Cannot add light:  maximum " << MAX_LIGHTS << " lights reached" << std::endl;
        delete light;
        return;
    }

    m_lights.emplace_back(light);
    std::cout << "✓ Light '" << light->getName() << "' added to scene (total: " << m_lights.size() << ")" << std::endl;
}

void LightManager::removeLight(const std::string &name)
{
    auto it = std::remove_if(m_lights.begin(), m_lights.end(),
                             [&name](const std::unique_ptr<Light> &light)
                             {
                                 return light->getName() == name;
                             });

    if (it != m_lights.end())
    {
        m_lights.erase(it, m_lights.end());
        std::cout << "✓ Light '" << name << "' removed from scene" << std::endl;
    }
}

Light *LightManager::findLight(const std::string &name)
{
    auto it = std::find_if(m_lights.begin(), m_lights.end(),
                           [&name](const std::unique_ptr<Light> &light)
                           {
                               return light->getName() == name;
                           });

    return it != m_lights.end() ? it->get() : nullptr;
}

uint32_t LightManager::getLightCount() const
{
    return static_cast<uint32_t>(
        std::count_if(m_lights.begin(), m_lights.end(),
                      [](const std::unique_ptr<Light> &light)
                      {
                          return light->isEnabled();
                      }));
}

void LightManager::setAmbientLight(const glm::vec3 &color, float intensity)
{
    m_ambientColor = color;
    m_ambientIntensity = intensity;
}

void LightManager::updateGPUBuffer()
{
    if (!m_lightBufferMapped)
    {
        return;
    }

    // Collecter les lumières actives
    std::vector<Light::GPUData> lightData;
    lightData.reserve(m_lights.size());

    for (const auto &light : m_lights)
    {
        if (light->isEnabled())
        {
            lightData.push_back(light->toGPUData());
        }
    }

    // Copier dans le buffer mappé
    if (!lightData.empty())
    {
        size_t dataSize = lightData.size() * sizeof(Light::GPUData);
        memcpy(m_lightBufferMapped, lightData.data(), dataSize);
    }
}

void LightManager::createLightBuffer()
{
    // Calculer la taille du buffer
    m_bufferSize = MAX_LIGHTS * sizeof(Light::GPUData);

    // Créer le buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device->getLogicalDevice(), &bufferInfo, nullptr, &m_lightBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create light buffer!");
    }

    // Allouer la mémoire
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->getLogicalDevice(), m_lightBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->findMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(m_device->getLogicalDevice(), &allocInfo, nullptr, &m_lightBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate light buffer memory!");
    }

    vkBindBufferMemory(m_device->getLogicalDevice(), m_lightBuffer, m_lightBufferMemory, 0);

    // Mapper la mémoire de façon persistante
    vkMapMemory(m_device->getLogicalDevice(), m_lightBufferMemory, 0, m_bufferSize, 0, &m_lightBufferMapped);
}

void LightManager::destroyLightBuffer()
{
    if (m_lightBufferMapped)
    {
        vkUnmapMemory(m_device->getLogicalDevice(), m_lightBufferMemory);
        m_lightBufferMapped = nullptr;
    }

    if (m_lightBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device->getLogicalDevice(), m_lightBuffer, nullptr);
        m_lightBuffer = VK_NULL_HANDLE;
    }

    if (m_lightBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device->getLogicalDevice(), m_lightBufferMemory, nullptr);
        m_lightBufferMemory = VK_NULL_HANDLE;
    }
}