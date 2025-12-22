#include "ImGuiManager.hpp"
#include "ImGuiWindow.hpp"
#include "core/Device.hpp"
#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "rendering/RenderPass.hpp"

#include "../utils/external/imgui/imgui.h"
#include "../utils/external/imgui/backends/imgui_impl_glfw.h"
#include "../utils/external/imgui/backends/imgui_impl_vulkan.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

ImGuiManager::ImGuiManager(Instance* instance, Device* device, Window* window,
                           RenderPass* renderPass, uint32_t imageCount,
                           VkSampleCountFlagBits msaaSamples)
    : m_device(device)
    , m_window(window)
    , m_renderPass(renderPass)
    , m_imguiDescriptorPool(VK_NULL_HANDLE)
{
    std::cout << "\n╔══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              IMGUI INITIALIZATION                      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════╝\n" << std::endl;

    // 1. Créer le descriptor pool pour ImGui
    createDescriptorPool(imageCount);

    // 2. Initialiser ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

    std::cout << "✓ ImGui context created" << std::endl;

    // 3. Setup style
    setupImGuiStyle();

    // 4. Initialiser le backend GLFW
    if (!ImGui_ImplGlfw_InitForVulkan(window->getHandle(), true))
    {
        throw std::runtime_error("Failed to initialize ImGui GLFW backend");
    }
    std::cout << "✓ ImGui GLFW backend initialized" << std::endl;

    // 5. Initialiser le backend Vulkan
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_3;  // Ou VK_API_VERSION_1_2 selon ton setup
    initInfo.Instance = instance->getHandle();
    initInfo.PhysicalDevice = device->getPhysicalDevice();
    initInfo.Device = device->getLogicalDevice();
    initInfo.QueueFamily = device->getQueueFamilies().graphicsFamily.value();
    initInfo.Queue = device->getGraphicsQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_imguiDescriptorPool;
    initInfo.MinImageCount = imageCount;
    initInfo.ImageCount = imageCount;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = nullptr;
    
    // Configuration du pipeline pour le main viewport
    initInfo.PipelineInfoMain.RenderPass = renderPass->getHandle();
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = msaaSamples;

    if (!ImGui_ImplVulkan_Init(&initInfo))
    {
        throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
    }
    std::cout << "✓ ImGui Vulkan backend initialized" << std::endl;

    std::cout << "\n✓ ImGui initialization complete\n" << std::endl;
}

ImGuiManager::~ImGuiManager()
{
    std::cout << "Cleaning up ImGui..." << std::endl;

    // IMPORTANT: Attendre que le device ait fini tous les rendus avant de détruire ImGui
    m_device->waitIdle();

    // Nettoyer les fenêtres d'abord
    m_windows.clear();

    // Si une frame ImGui est en cours, la terminer proprement
    // On utilise Render() qui gère automatiquement l'état de la frame
    if (ImGui::GetCurrentContext() != nullptr && ImGui::GetFrameCount() > 0)
    {
        ImGui::Render();
    }

    // Shutdown dans l'ordre inverse de l'initialisation
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_imguiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_imguiDescriptorPool, nullptr);
    }

    std::cout << "✓ ImGui cleaned up" << std::endl;
}

void ImGuiManager::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render toutes les fenêtres actives
    for (auto& window : m_windows)
    {
        if (window->isVisible())
        {
            window->render();
        }
    }
}

void ImGuiManager::endFrame(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}

void ImGuiManager::addWindow(std::unique_ptr<ImGuiWindow> window)
{
    m_windows.push_back(std::move(window));
}

void ImGuiManager::removeWindow(const std::string& name)
{
    m_windows.erase(
        std::remove_if(m_windows.begin(), m_windows.end(),
                       [&name](const std::unique_ptr<ImGuiWindow>& w) {
                           return w->getName() == name;
                       }),
        m_windows.end());
}

void ImGuiManager::setWindowVisible(const std::string& name, bool visible)
{
    for (auto& window : m_windows)
    {
        if (window->getName() == name)
        {
            window->setVisible(visible);
            return;
        }
    }
}

void ImGuiManager::onSwapChainRecreate(uint32_t imageCount)
{
    // Recrée le descriptor pool avec la nouvelle taille
    if (m_imguiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device->getLogicalDevice(), m_imguiDescriptorPool, nullptr);
    }
    createDescriptorPool(imageCount);

    // ImGui Vulkan backend doit aussi être notifié
    ImGui_ImplVulkan_SetMinImageCount(imageCount);
}

bool ImGuiManager::wantsCaptureMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiManager::wantsCaptureKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiManager::createDescriptorPool(uint32_t imageCount)
{
    // ImGui a besoin d'un descriptor pool séparé avec suffisamment d'espace
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * imageCount;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(m_device->getLogicalDevice(), &poolInfo, nullptr,
                               &m_imguiDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create ImGui descriptor pool");
    }
}

void ImGuiManager::setupImGuiStyle()
{
    // Style Dark par défaut avec quelques ajustements
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.FramePadding = ImVec2(5, 3);
    style.ItemSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 3.0f;

    // Couleurs personnalisées (optionnel)
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.27f, 0.32f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.37f, 0.42f, 1.00f);
}
