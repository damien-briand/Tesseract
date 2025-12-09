#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

class GameObject;
class Device;

/**
 * @brief Gère une collection d'objets 3D (scene graph simple)
 */
class Scene {
public:
    Scene(Device* device);
    ~Scene();

    // Suppression de la copie
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    /**
     * @brief Crée un nouvel objet dans la scène
     * @param name Nom de l'objet
     * @return Pointeur vers le GameObject créé
     */
    GameObject* createGameObject(const std::string& name);

    /**
     * @brief Trouve un objet par son nom
     * @param name Nom de l'objet
     * @return Pointeur vers l'objet ou nullptr
     */
    GameObject* findObjectByName(const std::string& name);

    /**
     * @brief Supprime un objet de la scène
     * @param name Nom de l'objet à supprimer
     */
    void removeObject(const std::string& name);

    /**
     * @brief Dessine tous les objets visibles de la scène
     * @param commandBuffer Command buffer Vulkan
     */
    void drawAll(VkCommandBuffer commandBuffer);

    /**
     * @brief Retourne tous les GameObjects
     */
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return m_objects; }

    /**
     * @brief Nombre d'objets dans la scène
     */
    size_t getObjectCount() const { return m_objects.size(); }

private:
    Device* m_device;
    std::vector<std::unique_ptr<GameObject>> m_objects;
};