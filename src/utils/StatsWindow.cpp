#include "StatsWindow.hpp"
#include "rendering/AdvancedCommandRecorder.hpp"
#include "scene/LightManager.hpp"
#include "scene/Scene.hpp"
#include "external/imgui/imgui.h"

StatsWindow::StatsWindow(AdvancedCommandRecorder* commandRecorder,
                         LightManager* lightManager,
                         Scene* scene)
    : ImGuiWindow("Stats")
    , m_commandRecorder(commandRecorder)
    , m_lightManager(lightManager)
    , m_scene(scene)
    , m_fps(0.0f)
    , m_frameTime(0.0f)
{
}

void StatsWindow::render()
{
    ImGui::Begin("Performance Stats", &m_visible);

    // Section FPS
    ImGui::Text("Frame Statistics");
    ImGui::Separator();
    
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::SameLine(150);
    ImGui::Text("Frame Time: %.2f ms", m_frameTime);

    // Graphique FPS
    ImGui::PlotLines("FPS History", m_fpsHistory, HISTORY_SIZE,
                     m_historyIndex, nullptr, 0.0f, 200.0f,
                     ImVec2(0, 80));

    // Graphique Frame Time
    ImGui::PlotLines("Frame Time (ms)", m_frameTimeHistory, HISTORY_SIZE,
                     m_historyIndex, nullptr, 0.0f, 33.0f,
                     ImVec2(0, 80));

    ImGui::Spacing();
    ImGui::Separator();

    // Section Scene
    ImGui::Text("Scene Information");
    ImGui::Separator();

    const auto& stats = m_commandRecorder->getStats();
    ImGui::Text("Objects Rendered: %u", stats.objectsRendered);
    ImGui::Text("Total Objects: %zu", m_scene->getObjectCount());
    ImGui::Text("Lights: %u", m_lightManager->getLightCount());

    ImGui::Spacing();
    ImGui::Separator();

    // Section Threading
    ImGui::Text("Threading");
    ImGui::Separator();
    ImGui::Text("Threads Used: %u", stats.threadsUsed);
    ImGui::Text("Last Record Time: %.2f ms", stats.lastRecordTime);

    ImGui::Spacing();
    ImGui::Separator();

    // Informations système
    ImGui::Text("System");
    ImGui::Separator();
    ImGui::Text("Application: Tesseract Engine");
    ImGui::Text("API: Vulkan 1.3");

    ImGui::End();
}

void StatsWindow::updateStats(float fps, float frameTime)
{
    m_fps = fps;
    m_frameTime = frameTime;
    updateHistory();
}

void StatsWindow::updateHistory()
{
    m_fpsHistory[m_historyIndex] = m_fps;
    m_frameTimeHistory[m_historyIndex] = m_frameTime;
    m_historyIndex = (m_historyIndex + 1) % HISTORY_SIZE;
}
