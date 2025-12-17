#include "SceneInspector.hpp"
#include "scene/Scene.hpp"
#include "scene/GameObject.hpp"
#include "external/imgui/imgui.h"

#include <glm/glm.hpp>

SceneInspector::SceneInspector(Scene* scene)
    : ImGuiWindow("Scene Inspector")
    , m_scene(scene)
    , m_selectedObjectIndex(-1)
{
}

void SceneInspector::render()
{
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(m_name.c_str(), &m_visible))
    {
        // Afficher le nombre d'objets dans la scène
        ImGui::Text("Objects in scene: %zu", m_scene->getObjectCount());
        ImGui::Separator();

        // Split en deux colonnes : liste d'objets à gauche, propriétés à droite
        float leftPanelWidth = 150.0f;
        
        // Panneau de gauche : liste des objets
        ImGui::BeginChild("ObjectList", ImVec2(leftPanelWidth, 0), true);
        renderObjectList();
        ImGui::EndChild();

        ImGui::SameLine();

        // Panneau de droite : propriétés de l'objet sélectionné
        ImGui::BeginChild("Properties", ImVec2(0, 0), true);
        if (m_selectedObjectIndex >= 0 && m_selectedObjectIndex < static_cast<int>(m_scene->getObjectCount()))
        {
            const auto& objects = m_scene->getObjects();
            GameObject* selectedObject = objects[m_selectedObjectIndex].get();
            renderObjectProperties(selectedObject);
        }
        else
        {
            ImGui::TextDisabled("No object selected");
            ImGui::TextWrapped("Select an object from the list to inspect and modify its properties.");
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void SceneInspector::renderObjectList()
{
    ImGui::Text("Objects");
    ImGui::Separator();

    const auto& objects = m_scene->getObjects();
    
    for (size_t i = 0; i < objects.size(); ++i)
    {
        const GameObject* obj = objects[i].get();
        
        // Couleur différente si l'objet est sélectionné
        bool isSelected = (static_cast<int>(i) == m_selectedObjectIndex);
        
        if (isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
        }

        // Afficher le nom de l'objet comme bouton sélectionnable
        std::string label = obj->getName();
        if (!obj->isVisible())
        {
            label += " (hidden)";
        }

        if (ImGui::Selectable(label.c_str(), isSelected))
        {
            m_selectedObjectIndex = static_cast<int>(i);
        }

        if (isSelected)
        {
            ImGui::PopStyleColor();
        }
    }
}

void SceneInspector::renderObjectProperties(GameObject* object)
{
    if (!object) return;

    ImGui::Text("Object: %s", object->getName().c_str());
    ImGui::Separator();

    // Visibility toggle
    bool visible = object->isVisible();
    if (ImGui::Checkbox("Visible", &visible))
    {
        object->setVisible(visible);
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === TRANSFORM ===
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Position
        glm::vec3 position = object->getPosition();
        float pos[3] = { position.x, position.y, position.z };
        
        if (ImGui::DragFloat3("Position", pos, 0.1f, -100.0f, 100.0f, "%.2f"))
        {
            object->setPosition(glm::vec3(pos[0], pos[1], pos[2]));
        }

        // Rotation (degrés)
        glm::vec3 rotation = object->getRotation();
        float rot[3] = { rotation.x, rotation.y, rotation.z };
        
        if (ImGui::DragFloat3("Rotation", rot, 1.0f, -360.0f, 360.0f, "%.1f°"))
        {
            object->setRotation(glm::vec3(rot[0], rot[1], rot[2]));
        }

        // Scale
        glm::vec3 scale = object->getScale();
        float scl[3] = { scale.x, scale.y, scale.z };
        
        if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.01f, 10.0f, "%.2f"))
        {
            object->setScale(glm::vec3(scl[0], scl[1], scl[2]));
        }

        // Uniform scale helper
        ImGui::SameLine();
        if (ImGui::Button("Uniform"))
        {
            float avgScale = (scl[0] + scl[1] + scl[2]) / 3.0f;
            object->setUniformScale(avgScale);
        }

        // Reset buttons
        ImGui::Spacing();
        if (ImGui::Button("Reset Position"))
        {
            object->setPosition(glm::vec3(0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Rotation"))
        {
            object->setRotation(glm::vec3(0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Scale"))
        {
            object->setScale(glm::vec3(1.0f));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === MESH INFO ===
    if (ImGui::CollapsingHeader("Mesh Info"))
    {
        ImGui::Text("Index count: %u", object->getIndexCount());
        ImGui::Text("Triangle count: %u", object->getIndexCount() / 3);
        ImGui::Text("Can draw: %s", object->canDraw() ? "Yes" : "No");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // === MATERIAL INFO ===
    if (ImGui::CollapsingHeader("Material"))
    {
        if (object->getMaterial())
        {
            ImGui::Text("Material assigned: Yes");
            // Ici on pourrait ajouter plus d'infos sur le material
            // (textures, propriétés PBR, etc.)
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No material assigned");
        }
    }
}
