#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "core/Camera.hpp"
#include "core/ThreadPool.hpp"

#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "rendering/AdvancedCommandRecorder.hpp"
#include "rendering/Buffer/UniformBuffer.hpp"
#include "rendering/Descriptor/DescriptorSetLayout.hpp"
#include "rendering/Descriptor/DescriptorPool.hpp"
#include "rendering/Material/Material.hpp"
#include "rendering/DepthBuffer.hpp"
#include "rendering/MSAABuffer.hpp"

#include "scene/Scene.hpp"
#include "scene/GameObject.hpp"
#include "scene/Light.hpp"       
#include "scene/LightManager.hpp"

#include "ui/ImGuiManager.hpp"
#include "ui/StatsWindow.hpp"
#include "ui/SceneInspector.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <iomanip>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;
const std::string SHADERS_PATH = "../src/shaders/spv/";
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Application
{
public:
    void run()
    {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // Core objects
    Window *m_window = nullptr;
    Instance *m_instance = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    Device *m_device = nullptr;
    SwapChain *m_swapChain = nullptr;
    Camera *m_camera = nullptr;
    ThreadPool *m_threadPool = nullptr;

    // Rendering objects
    RenderPass *m_renderPass = nullptr;
    DescriptorSetLayout *m_descriptorSetLayout = nullptr;
    Pipeline *m_pipeline = nullptr; // PBR pipeline
    Pipeline *m_gizmoPipeline = nullptr; // Unlit pipeline for gizmo
    AdvancedCommandRecorder *m_commandRecorder = nullptr;
    UniformBuffer *m_uniformBuffer = nullptr;
    Material *m_material = nullptr;
    DepthBuffer *m_depthBuffer = nullptr;
    DescriptorPool *m_descriptorPool = nullptr;
    Renderer *m_renderer = nullptr;

    MSAABuffer *m_msaaBuffer = nullptr;
    VkSampleCountFlagBits m_msaaSamples;

    // Scene
    Scene *m_scene = nullptr;
    LightManager *m_lightManager = nullptr; 

    // ImGui
    ImGuiManager *m_imguiManager = nullptr;
    StatsWindow *m_statsWindow = nullptr;
    SceneInspector *m_sceneInspector = nullptr;

    // Statistiques
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    uint32_t m_frameCount = 0;

    void initVulkan()
    {
        printHeader("TESSERACT ENGINE");

        // 1-13.  Initialisation standard (identique)
        m_window = new Window(WIDTH, HEIGHT, "Tesseract Engine");
        m_instance = new Instance("Tesseract", enableValidationLayers);
        m_surface = m_window->createSurface(m_instance->getHandle());
        m_threadPool = new ThreadPool(0);

        float aspectRatio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
        m_camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), 15.0f, aspectRatio);

        m_device = new Device(m_instance->getHandle(), m_surface);
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        m_swapChain = new SwapChain(m_device, m_surface, m_window);
        m_msaaBuffer = new MSAABuffer(m_device, m_swapChain, m_msaaSamples);
        m_depthBuffer = new DepthBuffer(m_device, m_swapChain, m_msaaSamples);

        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat(),
                                      m_depthBuffer->getFormat(), m_msaaSamples);

        m_swapChain->createFramebuffers(m_renderPass->getHandle(),
                                        m_msaaBuffer->getImageView(),
                                        m_depthBuffer->getImageView());

        m_descriptorSetLayout = new DescriptorSetLayout(m_device);


        // Pipeline PBR
        m_pipeline = new Pipeline(m_device, m_renderPass, m_descriptorSetLayout,
                      m_swapChain->getExtent(),
                      SHADERS_PATH + "vert.spv",
                      SHADERS_PATH + "frag.spv",
                      m_msaaSamples);

        // Pipeline unlit pour le gizmo
        m_gizmoPipeline = new Pipeline(m_device, m_renderPass, m_descriptorSetLayout,
                           m_swapChain->getExtent(),
                           SHADERS_PATH + "gizmo_vert.spv",
                           SHADERS_PATH + "gizmo_frag.spv",
                           m_msaaSamples,
                           false, false); // depthTest=false, depthWrite=false

        m_commandRecorder = new AdvancedCommandRecorder(m_device, m_threadPool, MAX_FRAMES_IN_FLIGHT, m_gizmoPipeline);
        m_uniformBuffer = new UniformBuffer(m_device, MAX_FRAMES_IN_FLIGHT);
        m_material = new Material(m_device,
                                  "../assets/models/Cube/Color.png",
                                  "../assets/models/Cube/Normal.png",
                                  "../assets/models/Cube/Metalic.png",
                                  "");

        // Créer le Light Manager
        m_lightManager = new LightManager(m_device);
        setupLights();

        // Créer le descriptor pool AVEC le light manager
        m_descriptorPool = new DescriptorPool(m_device, m_descriptorSetLayout,
                                              m_uniformBuffer, m_material,
                                              m_lightManager, 
                                              MAX_FRAMES_IN_FLIGHT);

        // Créer la scène
        m_scene = new Scene(m_device);
        createLargeScene();
        
        // Créer un gizmo d'axes à l'origine
        m_scene->createAxisGizmo(m_material, 1.0f, 0.02f);

        // Créer le renderer AVEC le light manager (sans ImGui d'abord)
        m_renderer = new Renderer(m_device, m_swapChain, m_window,
                      m_renderPass, m_pipeline, m_gizmoPipeline, m_commandRecorder,
                      m_uniformBuffer, m_descriptorPool,
                      m_msaaBuffer, m_depthBuffer,
                      MAX_FRAMES_IN_FLIGHT, m_camera, m_scene,
                      m_lightManager, nullptr); 

        // Initialiser ImGui (après le renderer)
        m_imguiManager = new ImGuiManager(m_instance, m_device, m_window,
                                          m_renderPass, MAX_FRAMES_IN_FLIGHT,
                                          m_msaaSamples);

        // Donner ImGuiManager au Renderer
        m_renderer->setImGuiManager(m_imguiManager);

        // Créer et ajouter les fenêtres ImGui
        m_statsWindow = new StatsWindow(m_commandRecorder, m_lightManager, m_scene);
        m_imguiManager->addWindow(std::unique_ptr<ImGuiWindow>(m_statsWindow));

        m_sceneInspector = new SceneInspector(m_scene);
        m_imguiManager->addWindow(std::unique_ptr<ImGuiWindow>(m_sceneInspector));

        printSummary();
    }

    void setupLights()
    {
        std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║              LIGHTING SETUP                            ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════╝\n"
                  << std::endl;

        // Lumière ambiante globale (très faible)
        m_lightManager->setAmbientLight(glm::vec3(0.03f, 0.03f, 0.04f), 1.0f);
        std::cout << "✓ Ambient light configured" << std::endl;

        // Lumière directionnelle principale (soleil)
        Light *sun = Light::createDirectional(
            "Sun",
            glm::vec3(-0.3f, -1.0f, -0.5f), // Direction
            glm::vec3(1.0f, 0.95f, 0.9f),   // Couleur légèrement jaune
            2.0f                            // Intensité
        );
        m_lightManager->addLight(sun);

        // Lumière ponctuelle 1 (lampe bleue)
        Light *blueLight = Light::createPoint(
            "Blue Light",
            glm::vec3(-10.0f, 5.0f, 0.0f), // Position
            glm::vec3(0.2f, 0.4f, 1.0f),   // Couleur bleue
            50.0f,                         // Intensité
            30.0f                          // Range
        );
        m_lightManager->addLight(blueLight);

        // Lumière ponctuelle 2 (lampe rouge)
        Light *redLight = Light::createPoint(
            "Red Light",
            glm::vec3(10.0f, 5.0f, 0.0f), // Position
            glm::vec3(1.0f, 0.2f, 0.2f),  // Couleur rouge
            200.0f,                        // Intensité
            30.0f                         // Range
        );
        m_lightManager->addLight(redLight);

        // Lumière ponctuelle 3 (lampe verte au centre, en hauteur)
        Light *greenLight = Light::createPoint(
            "Green Light",
            glm::vec3(0.0f, 15.0f, 0.0f), // Position au-dessus
            glm::vec3(0.2f, 1.0f, 0.3f),  // Couleur verte
            100.0f,                       // Intensité
            40.0f                         // Range
        );
        m_lightManager->addLight(greenLight);

        std::cout << "\n✓ Lighting setup complete:" << std::endl;
        std::cout << "  - 1 directional light (sun)" << std::endl;
        std::cout << "  - 3 point lights (colored)" << std::endl;
        std::cout << "  - Ambient lighting enabled" << std::endl;
    }

    void createLargeScene()
    {
        const int GRID_SIZE = 3; // 3x3 = 9 objets
        const float SPACING = 4.0f;

        std::cout << "\nCreating scene (" << GRID_SIZE << "x" << GRID_SIZE << " = "
                  << (GRID_SIZE * GRID_SIZE) << " objects)..." << std::endl;

        for (int x = 0; x < GRID_SIZE; x++)
        {
            for (int z = 0; z < GRID_SIZE; z++)
            {
                std::string name = "Cube_" + std::to_string(x) + "_" + std::to_string(z);
                GameObject *cube = m_scene->createGameObject(name);
                cube->loadMeshFromFile("../assets/models/Cube/Cube.obj");

                float posX = (x - GRID_SIZE / 2.0f) * SPACING;
                float posZ = (z - GRID_SIZE / 2.0f) * SPACING;
                float posY = std::sin(x * 0.4f) * std::cos(z * 0.4f) * 3.0f;
                cube->setPosition(glm::vec3(posX, posY, posZ));

                cube->setRotation(glm::vec3(x * 5.0f, z * 5.0f, (x + z) * 3.0f));

                float scale = 0.7f + (std::sin(x * 0.3f + z * 0.3f) * 0.2f);
                cube->setUniformScale(scale);

                cube->setMaterial(m_material);
            }
        }

        std::cout << "✓ Scene created successfully" << std::endl;
    }

    void mainLoop()
    {
        printHeader("MAIN LOOP STARTED");

        auto lastTime = std::chrono::high_resolution_clock::now();
        auto lastFPSTime = lastTime;

        while (!m_window->shouldClose())
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(
                                  currentTime - lastTime)
                                  .count();
            lastTime = currentTime;

            m_frameCount++;
            float timeSinceFPS = std::chrono::duration<float, std::chrono::seconds::period>(
                                     currentTime - lastFPSTime)
                                     .count();

            if (timeSinceFPS >= 1.0f)
            {
                m_fps = m_frameCount / timeSinceFPS;
                m_frameTime = timeSinceFPS / m_frameCount * 1000.0f;

                // Mettre à jour les stats ImGui
                m_statsWindow->updateStats(m_fps, m_frameTime);

                const auto &stats = m_commandRecorder->getStats();

                /*
                std::cout << "\r"
                << "FPS:  " << std::setw(6) << std::fixed << std::setprecision(1) << m_fps
                << " | Frame:  " << std::setw(5) << std::setprecision(2) << m_frameTime << "ms"
                << " | Objects: " << std::setw(4) << stats.objectsRendered
                << " | Lights: " << std::setw(2) << m_lightManager->getLightCount()
                << " | Threads: " << stats.threadsUsed
                << std::flush;
                */

                m_frameCount = 0;
                lastFPSTime = currentTime;
            }

            m_window->pollEvents();
            m_camera->update(m_window, deltaTime);
            m_renderer->drawFrame();
        }

        std::cout << std::endl;
        printHeader("MAIN LOOP ENDED");

        m_renderer->waitIdle();
    }

    void cleanup()
    {
        std::cout << "\nStarting cleanup..." << std::endl;

        delete m_imguiManager;  // Nettoyer ImGui en premier (doit être avant le renderPass)
        m_statsWindow = nullptr;  // Déjà détruit par m_imguiManager
        m_sceneInspector = nullptr;  // Déjà détruit par m_imguiManager

        delete m_renderer;
        delete m_scene;
        delete m_lightManager; 
        delete m_descriptorPool;
        delete m_material;
        delete m_uniformBuffer;
        delete m_commandRecorder;
        delete m_pipeline;
        delete m_gizmoPipeline;
        delete m_descriptorSetLayout;
        delete m_renderPass;
        delete m_swapChain;
        delete m_msaaBuffer;
        delete m_depthBuffer;

        vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);

        delete m_camera;

        if (m_threadPool)
        {
            delete m_threadPool;
        }

        delete m_device;
        delete m_instance;
        delete m_window;

        printHeader("CLEANUP COMPLETED");
    }

    void printHeader(const std::string &title)
    {
        std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  " << std::left << std::setw(52) << title << "  ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    }

    void printSummary()
    {
        std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║              INITIALIZATION SUMMARY                    ║" << std::endl;
        std::cout << "╠════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  Objects:               " << std::setw(28) << std::right << m_scene->getObjectCount() << "   ║" << std::endl;
        std::cout << "║  Lights:               " << std::setw(28) << m_lightManager->getLightCount() << "    ║" << std::endl;
        std::cout << "║  Worker threads:       " << std::setw(28) << m_threadPool->getThreadCount() << "    ║" << std::endl;
        std::cout << "║  MSAA samples:         " << std::setw(28) << m_msaaSamples << "    ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;
    }
};

int main()
{
    Application app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n✗ FATAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}