#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class Device;
class Window;
class Instance;
class RenderPass;
class ImGuiWindow;

/**
 * @brief Gère l'intégration d'ImGui dans le moteur Vulkan
 * 
 * Responsabilités :
 * - Initialiser ImGui avec les backends GLFW et Vulkan
 * - Gérer le descriptor pool ImGui
 * - Enregistrer les commandes de rendu ImGui
 * - Gérer les fenêtres ImGui personnalisées
 */
class ImGuiManager
{
public:
    /**
     * @brief Constructeur
     * @param instance Instance Vulkan
     * @param device Device Vulkan
     * @param window Fenêtre GLFW
     * @param renderPass RenderPass où ImGui sera rendu
     * @param imageCount Nombre d'images dans la swapchain
     * @param msaaSamples Nombre d'échantillons MSAA
     */
    ImGuiManager(Instance* instance, Device* device, Window* window,
                 RenderPass* renderPass, uint32_t imageCount,
                 VkSampleCountFlagBits msaaSamples);
    ~ImGuiManager();

    // Suppression de la copie
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

    /**
     * @brief Commence une nouvelle frame ImGui
     * Doit être appelé avant tout appel ImGui::Begin()
     */
    void beginFrame();

    /**
     * @brief Finalise la frame et enregistre les commandes de rendu
     * @param commandBuffer Command buffer où enregistrer les commandes ImGui
     */
    void endFrame(VkCommandBuffer commandBuffer);

    /**
     * @brief Ajoute une fenêtre ImGui personnalisée
     * @param window Fenêtre à ajouter (le manager prend possession)
     */
    void addWindow(std::unique_ptr<ImGuiWindow> window);

    /**
     * @brief Retire une fenêtre par son nom
     */
    void removeWindow(const std::string& name);

    /**
     * @brief Active/désactive une fenêtre
     */
    void setWindowVisible(const std::string& name, bool visible);

    /**
     * @brief Recrée les ressources lors d'un resize de swapchain
     */
    void onSwapChainRecreate(uint32_t imageCount);

    /**
     * @brief Vérifie si ImGui veut capturer la souris
     */
    bool wantsCaptureMouse() const;

    /**
     * @brief Vérifie si ImGui veut capturer le clavier
     */
    bool wantsCaptureKeyboard() const;

private:
    Device* m_device;
    Window* m_window;
    RenderPass* m_renderPass;

    VkDescriptorPool m_imguiDescriptorPool;

    std::vector<std::unique_ptr<ImGuiWindow>> m_windows;

    void createDescriptorPool(uint32_t imageCount);
    void setupImGuiStyle();
};
