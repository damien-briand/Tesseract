#pragma once

#include "ImGuiWindow.hpp"

class AdvancedCommandRecorder;
class LightManager;
class Scene;

/**
 * @brief Fenêtre ImGui affichant les statistiques de performance du moteur
 * 
 * Affiche :
 * - FPS et temps de frame
 * - Nombre d'objets/lumières
 * - Utilisation des threads
 * - Informations GPU
 */
class StatsWindow : public ImGuiWindow
{
public:
    StatsWindow(AdvancedCommandRecorder* commandRecorder,
                LightManager* lightManager,
                Scene* scene);

    void render() override;

    /**
     * @brief Met à jour les statistiques (appelé depuis main loop)
     */
    void updateStats(float fps, float frameTime);

private:
    AdvancedCommandRecorder* m_commandRecorder;
    LightManager* m_lightManager;
    Scene* m_scene;

    // Stats
    float m_fps;
    float m_frameTime;

    // Historique pour graphiques
    static constexpr int HISTORY_SIZE = 90;
    float m_fpsHistory[HISTORY_SIZE] = {};
    float m_frameTimeHistory[HISTORY_SIZE] = {};
    int m_historyIndex = 0;

    void updateHistory();
};
