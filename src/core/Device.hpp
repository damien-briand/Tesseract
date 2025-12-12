#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device
{
public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    // Suppression de la copie
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    // Getters
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    VkDevice getLogicalDevice() const { return m_device; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue getPresentQueue() const { return m_presentQueue; }
    QueueFamilyIndices getQueueFamilies() const { return m_queueFamilyIndices; }

    /**
     * @brief Obtient le nombre maximum de samples MSAA supportés
     * @return VkSampleCountFlagBits (VK_SAMPLE_COUNT_1_BIT à VK_SAMPLE_COUNT_64_BIT)
     */
    VkSampleCountFlagBits getMaxUsableSampleCount() const;

    /**
     * @brief Trouve un type de mémoire compatible avec les requirements
     * @param typeFilter Filtre des types de mémoire
     * @param properties Propriétés requises
     * @return Index du type de mémoire trouvé
     */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Méthodes utilitaires
    void waitIdle() const;

private:
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    QueueFamilyIndices m_queueFamilyIndices;

    VkInstance m_instance;
    VkSurfaceKHR m_surface;

    std::vector<const char *> m_deviceExtensions;

    // Méthodes privées
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};