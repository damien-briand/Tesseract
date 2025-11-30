#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

class Device;

class ShaderLoader {
public:
    // Suppression du constructeur (classe statique)
    ShaderLoader() = delete;

    /**
     * @brief Charge un shader SPIR-V et crée un VkShaderModule
     * @param device Pointeur vers le Device
     * @param filename Chemin vers le fichier .spv
     * @return VkShaderModule créé
     * @throws std::runtime_error si le chargement échoue
     */
    static VkShaderModule loadShader(VkDevice device, const std::string& filename);

    /**
     * @brief Crée un VkShaderModule à partir de code SPIR-V
     * @param device Device Vulkan
     * @param code Code SPIR-V
     * @return VkShaderModule créé
     * @throws std::runtime_error si la création échoue
     */
    static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

    /**
     * @brief Détruit un VkShaderModule
     * @param device Device Vulkan
     * @param shaderModule Module à détruire
     */
    static void destroyShaderModule(VkDevice device, VkShaderModule shaderModule);

    /**
     * @brief Vérifie si un fichier shader existe et est valide
     * @param filename Chemin vers le fichier
     * @return true si le fichier est valide
     */
    static bool isValidShaderFile(const std::string& filename);

private:
    // Magic number SPIR-V (0x07230203)
    static constexpr uint32_t SPIRV_MAGIC_NUMBER = 0x07230203;

    /**
     * @brief Vérifie si le code binaire est du SPIR-V valide
     * @param code Code à vérifier
     * @return true si c'est du SPIR-V valide
     */
    static bool isValidSpirV(const std::vector<char>& code);
};