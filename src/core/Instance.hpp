#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Instance {
public:
    Instance(const std::string& appName, bool enableValidation);
    ~Instance();

    // Suppression de la copie
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    // Getters
    VkInstance getHandle() const { return m_instance; }
    bool hasValidationLayers() const { return m_enableValidationLayers; }

private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    bool m_enableValidationLayers;

    std::vector<const char*> m_validationLayers;

    // Méthodes privées
    void createInstance(const std::string& appName);
    void setupDebugMessenger();
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // Debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};