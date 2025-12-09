#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <memory>
#include <vector>

class Material;
class VertexBuffer;
class IndexBuffer;
class Device;
struct Vertex;

/**
 * @brief Représente un objet 3D dans la scène avec mesh, transform et material
 */
class GameObject {
public:
    /**
     * @brief Constructeur
     * @param name Nom de l'objet (pour debug)
     * @param device Pointeur vers le device Vulkan
     */
    GameObject(const std::string& name, Device* device);
    ~GameObject();

    // Suppression de la copie
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    // ========================================================================
    // MESH DATA
    // ========================================================================
    
    /**
     * @brief Définit les données de mesh (vertices + indices)
     * @param vertices Vertices du mesh
     * @param indices Indices du mesh
     */
    void setMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    
    /**
     * @brief Charge un mesh depuis un fichier OBJ
     * @param filepath Chemin vers le fichier . obj
     */
    bool loadMeshFromFile(const std::string& filepath);

    // ========================================================================
    // TRANSFORM
    // ========================================================================
    
    // Position
    void setPosition(const glm::vec3& position) { m_position = position; m_transformDirty = true; }
    const glm::vec3& getPosition() const { return m_position; }
    void translate(const glm::vec3& offset) { m_position += offset; m_transformDirty = true; }

    // Rotation (Euler angles en degrés)
    void setRotation(const glm::vec3& rotation) { m_rotation = rotation; m_transformDirty = true; }
    const glm::vec3& getRotation() const { return m_rotation; }
    void rotate(const glm::vec3& delta) { m_rotation += delta; m_transformDirty = true; }

    // Scale
    void setScale(const glm::vec3& scale) { m_scale = scale; m_transformDirty = true; }
    const glm::vec3& getScale() const { return m_scale; }
    void setUniformScale(float scale) { m_scale = glm::vec3(scale); m_transformDirty = true; }

    /**
     * @brief Retourne la matrice de transformation Model (Position * Rotation * Scale)
     */
    glm::mat4 getModelMatrix();

    // ========================================================================
    // MATERIAL
    // ========================================================================
    
    void setMaterial(Material* material) { m_material = material; }
    Material* getMaterial() const { return m_material; }

    // ========================================================================
    // RENDERING
    // ========================================================================
    
    /**
     * @brief Dessine cet objet dans un command buffer
     * @param commandBuffer Command buffer Vulkan
     */
    void draw(VkCommandBuffer commandBuffer);

    /**
     * @brief Vérifie si l'objet a un mesh et peut être rendu
     */
    bool canDraw() const { return m_vertexBuffer != nullptr && m_indexBuffer != nullptr; }

    // Getters
    const std::string& getName() const { return m_name; }
    uint32_t getIndexCount() const;

    // Visibility
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

private:
    std::string m_name;
    Device* m_device;

    // Mesh data
    VertexBuffer* m_vertexBuffer;
    IndexBuffer* m_indexBuffer;
    uint32_t m_indexCount;

    // Transform
    glm::vec3 m_position;
    glm::vec3 m_rotation;  // Euler angles (degrés)
    glm::vec3 m_scale;
    glm::mat4 m_modelMatrix;
    bool m_transformDirty;  // Si true, recalculer la matrice

    // Material
    Material* m_material;  // Pointeur non-owning

    // Visibility
    bool m_visible;

    void updateModelMatrix();
};