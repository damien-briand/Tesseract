#pragma once

#include "Light.hpp"
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

class Device;

/**
 * @brief Gère toutes les lumières de la scène
 *
 * Responsabilités :
 * - Stocker et gérer les lumières
 * - Créer et mettre à jour le storage buffer GPU
 * - Fournir l'accès aux lumières pour le rendu
 */
class LightManager
{
public:
    explicit LightManager(Device *device);
    ~LightManager();

    LightManager(const LightManager &) = delete;
    LightManager &operator=(const LightManager &) = delete;

    /**
     * @brief Ajoute une lumière à la scène
     * @param light Lumière à ajouter (le manager prend possession)
     */
    void addLight(Light *light);

    /**
     * @brief Retire une lumière par son nom
     */
    void removeLight(const std::string &name);

    /**
     * @brief Trouve une lumière par son nom
     */
    Light *findLight(const std::string &name);

    /**
     * @brief Met à jour le storage buffer GPU avec les données actuelles
     */
    void updateGPUBuffer();

    /**
     * @brief Retourne le storage buffer pour binding
     */
    VkBuffer getBuffer() const { return m_lightBuffer; }

    /**
     * @brief Retourne le nombre de lumières actives
     */
    uint32_t getLightCount() const;

    /**
     * @brief Retourne toutes les lumières
     */
    const std::vector<std::unique_ptr<Light>> &getLights() const { return m_lights; }

    /**
     * @brief Définit la lumière ambiante globale
     */
    void setAmbientLight(const glm::vec3 &color, float intensity);
    glm::vec3 getAmbientColor() const { return m_ambientColor; }
    float getAmbientIntensity() const { return m_ambientIntensity; }

private:
    Device *m_device;
    std::vector<std::unique_ptr<Light>> m_lights;

    // Lumière ambiante (ne va pas dans le buffer, juste un terme constant)
    glm::vec3 m_ambientColor;
    float m_ambientIntensity;

    // GPU resources
    VkBuffer m_lightBuffer;
    VkDeviceMemory m_lightBufferMemory;
    void *m_lightBufferMapped;
    VkDeviceSize m_bufferSize;

    void createLightBuffer();
    void destroyLightBuffer();
};