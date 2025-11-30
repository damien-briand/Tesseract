#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class VulkanUtils {
public:
    // Suppression du constructeur (classe statique)
    VulkanUtils() = delete;

    /**
     * @brief Convertit un VkResult en string lisible
     * @param result Code VkResult
     * @return String représentant le résultat
     */
    static std::string resultToString(VkResult result);

    /**
     * @brief Affiche les informations d'un physical device
     * @param physicalDevice Device à inspecter
     */
    static void printPhysicalDeviceInfo(VkPhysicalDevice physicalDevice);

    /**
     * @brief Affiche les extensions disponibles sur une instance
     * @param instance Instance Vulkan
     */
    static void printAvailableExtensions();

    /**
     * @brief Affiche les layers de validation disponibles
     */
    static void printAvailableLayers();

    /**
     * @brief Retourne le nom du type de physical device
     * @param deviceType Type de device
     * @return String représentant le type
     */
    static std::string deviceTypeToString(VkPhysicalDeviceType deviceType);

    /**
     * @brief Retourne le nom d'un format Vulkan
     * @param format Format Vulkan
     * @return String représentant le format
     */
    static std::string formatToString(VkFormat format);

    /**
     * @brief Vérifie si un VkResult indique un succès
     * @param result Résultat à vérifier
     * @param errorMessage Message d'erreur si échec
     * @throws std::runtime_error si le résultat n'est pas VK_SUCCESS
     */
    static void checkResult(VkResult result, const std::string& errorMessage);

    /**
     * @brief Retourne la quantité de mémoire d'un physical device en MB
     * @param physicalDevice Device à inspecter
     * @return Mémoire totale en MB
     */
    static uint64_t getDeviceMemoryInMB(VkPhysicalDevice physicalDevice);
};