#include "GameObject.hpp"
#include "../rendering/Buffer/VertexBuffer.hpp"
#include "../rendering/Buffer/IndexBuffer.hpp"
#include "../rendering/Material/Material.hpp"
#include "../rendering/Vertex.hpp"
#include "../utils/ModelLoader.hpp"
#include <iostream>

// ============================================================================
// CONSTRUCTEUR / DESTRUCTEUR
// ============================================================================

GameObject::GameObject(const std::string &name, Device *device)
    : m_name(name), m_device(device), m_vertexBuffer(nullptr), m_indexBuffer(nullptr), m_indexCount(0), m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f), m_modelMatrix(1.0f), m_transformDirty(true), m_material(nullptr), m_visible(true)
{
}

GameObject::~GameObject()
{
    delete m_vertexBuffer;
    delete m_indexBuffer;
    // Note : m_material n'est pas supprimé ici (géré ailleurs)
}

// ============================================================================
// MESH DATA
// ============================================================================

void GameObject::setMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
    // Supprimer les anciens buffers si existants
    delete m_vertexBuffer;
    delete m_indexBuffer;

    // Créer les nouveaux buffers
    m_vertexBuffer = new VertexBuffer(m_device, vertices);
    m_indexBuffer = new IndexBuffer(m_device, indices);
    m_indexCount = static_cast<uint32_t>(indices.size());

    std::cout << "✓ Mesh set for '" << m_name << "' ("
              << vertices.size() << " vertices, "
              << indices.size() << " indices)" << std::endl;
}

bool GameObject::loadMeshFromFile(const std::string &filepath)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    if (!ModelLoader::loadModel(filepath, vertices, indices))
    {
        std::cerr << "✗ Failed to load mesh for '" << m_name << "' from " << filepath << std::endl;
        return false;
    }

    setMesh(vertices, indices);
    std::cout << "✓ Mesh loaded for '" << m_name << "' from " << filepath << std::endl;
    return true;
}

// ============================================================================
// TRANSFORM - Matrice Model
// ============================================================================

glm::mat4 GameObject::getModelMatrix()
{
    if (m_transformDirty)
    {
        updateModelMatrix();
        m_transformDirty = false;
    }
    return m_modelMatrix;
}

void GameObject::updateModelMatrix()
{
    // Ordre : Translation * Rotation * Scale (TRS)

    // 1. Scale
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

    // 2. Rotation (Euler angles :  Z * Y * X)
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    rotation = glm::rotate(rotation, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

    // 3. Translation
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);

    // Combinaison :  T * R * S
    m_modelMatrix = translation * rotation * scale;
}

// ============================================================================
// RENDERING
// ============================================================================

void GameObject::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    if (!canDraw() || !m_visible)
    {
        return;
    }

    // Binder les buffers
    m_vertexBuffer->bind(commandBuffer);
    m_indexBuffer->bind(commandBuffer);

    // Pousser la matrice Model de cet objet
    glm::mat4 modelMatrix = getModelMatrix();
    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,                 // offset
        sizeof(glm::mat4), // size
        &modelMatrix       // data
    );

    // Draw call
    vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
}

uint32_t GameObject::getIndexCount() const
{
    return m_indexCount;
}