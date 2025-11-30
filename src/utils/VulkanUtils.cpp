#include "VulkanUtils.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>

std::string VulkanUtils::resultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        default: return "UNKNOWN_ERROR (" + std::to_string(result) + ")";
    }
}

void VulkanUtils::printPhysicalDeviceInfo(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    std::cout << "\n========== Physical Device Info ==========" << std::endl;
    std::cout << "Device Name: " << properties.deviceName << std::endl;
    std::cout << "Device Type: " << deviceTypeToString(properties.deviceType) << std::endl;
    std::cout << "API Version: " 
              << VK_VERSION_MAJOR(properties.apiVersion) << "."
              << VK_VERSION_MINOR(properties.apiVersion) << "."
              << VK_VERSION_PATCH(properties.apiVersion) << std::endl;
    std::cout << "Driver Version: " << properties.driverVersion << std::endl;
    std::cout << "Vendor ID: 0x" << std::hex << properties.vendorID << std::dec << std::endl;
    std::cout << "Device ID: 0x" << std::hex << properties.deviceID << std::dec << std::endl;
    std::cout << "Memory: " << getDeviceMemoryInMB(physicalDevice) << " MB" << std::endl;
    std::cout << "Geometry Shader: " << (features.geometryShader ? "Yes" : "No") << std::endl;
    std::cout << "Tessellation Shader: " << (features.tessellationShader ? "Yes" : "No") << std::endl;
    std::cout << "=========================================\n" << std::endl;
}

void VulkanUtils::printAvailableExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "\n===== Available Vulkan Extensions =====" << std::endl;
    for (const auto& extension : extensions) {
        std::cout << "  • " << extension.extensionName << " (v" << extension.specVersion << ")" << std::endl;
    }
    std::cout << "Total: " << extensionCount << " extensions\n" << std::endl;
}

void VulkanUtils::printAvailableLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    std::cout << "\n====== Available Validation Layers ======" << std::endl;
    for (const auto& layer : layers) {
        std::cout << "  • " << layer.layerName << std::endl;
        std::cout << "    Description: " << layer.description << std::endl;
    }
    std::cout << "Total: " << layerCount << " layers\n" << std::endl;
}

std::string VulkanUtils::deviceTypeToString(VkPhysicalDeviceType deviceType) {
    switch (deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown";
    }
}

std::string VulkanUtils::formatToString(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
        case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
        case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
        case VK_FORMAT_D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        default: return "Format(" + std::to_string(format) + ")";
    }
}

void VulkanUtils::checkResult(VkResult result, const std::string& errorMessage) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(errorMessage + " (" + resultToString(result) + ")");
    }
}

uint64_t VulkanUtils::getDeviceMemoryInMB(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint64_t totalMemory = 0;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
        if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            totalMemory += memProperties.memoryHeaps[i].size;
        }
    }

    return totalMemory / (1024 * 1024); // Convertir en MB
}