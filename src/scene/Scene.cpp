#include "Scene.hpp"
#include "GameObject.hpp"
#include "../utils/PrimitiveGenerator.hpp"
#include <algorithm>
#include <iostream>

Scene::Scene(Device* device)
    : m_device(device)
{
}

Scene::~Scene() {
    // Les unique_ptr se nettoient automatiquement
}

GameObject* Scene::createGameObject(const std::string& name) {
    // Vérifier qu'un objet avec ce nom n'existe pas déjà
    if (findObjectByName(name) != nullptr) {
        std::cerr << "GameObject '" << name << "' already exists!" << std::endl;
        return findObjectByName(name);
    }

    m_objects.push_back(std::make_unique<GameObject>(name, m_device));
    std::cout << "✓ GameObject '" << name << "' created" << std::endl;
    return m_objects.back().get();
}

GameObject* Scene:: findObjectByName(const std:: string& name) {
    auto it = std::find_if(m_objects.begin(), m_objects.end(),
        [&name](const std::unique_ptr<GameObject>& obj) {
            return obj->getName() == name;
        });

    return (it != m_objects.end()) ? it->get() : nullptr;
}

void Scene::removeObject(const std::string& name) {
    auto it = std:: remove_if(m_objects.begin(), m_objects.end(),
        [&name](const std::unique_ptr<GameObject>& obj) {
            return obj->getName() == name;
        });

    if (it != m_objects.end()) {
        m_objects. erase(it, m_objects.end());
        std::cout << "✓ GameObject '" << name << "' removed" << std::endl;
    }
}

void Scene::drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    for (auto& obj : m_objects) {
        obj->draw(commandBuffer, pipelineLayout);
    }
}

GameObject* Scene::createAxisGizmo(Material* material, float length, float thickness) {
    std::cout << "Creating axis gizmo..." << std::endl;
    
    GameObject* gizmo = createGameObject("AxisGizmo");
    
    // Générer le mesh des axes
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    PrimitiveGenerator::generateAxisGizmo(length, thickness, vertices, indices);
    
    gizmo->setMesh(vertices, indices);
    gizmo->setMaterial(material);
    gizmo->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    
    std::cout << "✓ Axis gizmo created (X=red, Y=green, Z=blue)" << std::endl;
    
    return gizmo;
}

GameObject* Scene::createGridXZ(Material* material, float size) {
    GameObject* grid = createGameObject("GridXZ");
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    PrimitiveGenerator::generateXZQuad(size, vertices, indices);
    grid->setMesh(vertices, indices);
    if (material) grid->setMaterial(material);
    grid->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    return grid;
}