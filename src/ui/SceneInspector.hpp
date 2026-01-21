#pragma once

#include "ImGuiWindow.hpp"
#include <string>

class Scene;
class GameObject;

/**
 * @brief Fenêtre ImGui pour inspecter et modifier les objets de la scène
 */
class SceneInspector : public ImGuiWindow {
public:
    explicit SceneInspector(Scene* scene);
    ~SceneInspector() override = default;

    void render() override;

private:
    Scene* m_scene;
    int m_selectedObjectIndex;  // Index de l'objet sélectionné (-1 si aucun)

    void renderObjectList();
    void renderObjectProperties(GameObject* object);
};
