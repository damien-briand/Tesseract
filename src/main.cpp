#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "core/Camera.hpp"
#include "core/ThreadPool.hpp" // NOUVEAU

#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "rendering/AdvancedCommandRecorder.hpp" // NOUVEAU
#include "rendering/Buffer/UniformBuffer.hpp"
#include "rendering/Descriptor/DescriptorSetLayout.hpp"
#include "rendering/Descriptor/DescriptorPool.hpp"
#include "rendering/Material/Material.hpp"
#include "rendering/DepthBuffer.hpp"
#include "rendering/MSAABuffer.hpp"

#include "scene/Scene.hpp"
#include "scene/GameObject.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <iomanip>

const uint32_t WIDTH = 480;
const uint32_t HEIGHT = 480;
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
    ThreadPool *m_threadPool = nullptr; // NOUVEAU

    // Rendering objects
    RenderPass *m_renderPass = nullptr;
    DescriptorSetLayout *m_descriptorSetLayout = nullptr;
    Pipeline *m_pipeline = nullptr;
    AdvancedCommandRecorder *m_commandRecorder = nullptr; // NOUVEAU
    UniformBuffer *m_uniformBuffer = nullptr;
    Material *m_material = nullptr;
    DepthBuffer *m_depthBuffer = nullptr;
    DescriptorPool *m_descriptorPool = nullptr;
    Renderer *m_renderer = nullptr;

    MSAABuffer *m_msaaBuffer = nullptr;
    VkSampleCountFlagBits m_msaaSamples;

    // Scene
    Scene *m_scene = nullptr;

    // Statistiques
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    uint32_t m_frameCount = 0;

    void initVulkan()
    {
        printHeader("TESSERACT ENGINE - ADVANCED MULTITHREADING");

        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Tesseract - Advanced Multithreading");

        // 2. Créer l'instance Vulkan
        m_instance = new Instance("Tesseract", enableValidationLayers);

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());

        // 4. NOUVEAU : Créer le Thread Pool (0 = auto-detect CPU cores)
        m_threadPool = new ThreadPool(0);

        // 5. Créer la caméra
        float aspectRatio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
        m_camera = new Camera(
            glm::vec3(0.0f, 0.0f, 0.0f),
            15.0f,
            aspectRatio);

        // 6. Créer le device
        m_device = new Device(m_instance->getHandle(), m_surface);
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        // 7. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);

        // 8. Créer le buffer MSAA
        m_msaaBuffer = new MSAABuffer(m_device, m_swapChain, m_msaaSamples);

        // 9. Créer le depth buffer
        m_depthBuffer = new DepthBuffer(m_device, m_swapChain, m_msaaSamples);

        // 10. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat(),
                                      m_depthBuffer->getFormat(), m_msaaSamples);

        // 11. Créer les framebuffers
        m_swapChain->createFramebuffers(m_renderPass->getHandle(),
                                        m_msaaBuffer->getImageView(),
                                        m_depthBuffer->getImageView());

        // 12. Créer le descriptor set layout
        m_descriptorSetLayout = new DescriptorSetLayout(m_device);

        // 13. Créer le pipeline graphique
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_descriptorSetLayout,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv",
            m_msaaSamples);

        // 14. NOUVEAU : Créer l'Advanced Command Recorder
        m_commandRecorder = new AdvancedCommandRecorder(m_device, m_threadPool, MAX_FRAMES_IN_FLIGHT);

        // 15. Créer les uniform buffers
        m_uniformBuffer = new UniformBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 16. Charger le material
        m_material = new Material(m_device,
                                  "../assets/models/Cube/Color.png",
                                  "../assets/models/Cube/Normal.png",
                                  "../assets/models/Cube/Metalic.png",
                                  "");

        // 17. Créer le descriptor pool
        m_descriptorPool = new DescriptorPool(
            m_device,
            m_descriptorSetLayout,
            m_uniformBuffer,
            m_material,
            MAX_FRAMES_IN_FLIGHT);

        // 18. Créer la scène
        m_scene = new Scene(m_device);
        createLargeScene();

        // 19. Créer le renderer
        m_renderer = new Renderer(
            m_device,
            m_swapChain,
            m_window,
            m_renderPass,
            m_pipeline,
            m_commandRecorder,
            m_uniformBuffer,
            m_descriptorPool,
            m_msaaBuffer,
            m_depthBuffer,
            MAX_FRAMES_IN_FLIGHT,
            m_camera,
            m_scene);

        printSummary();
    }

    void createLargeScene()
    {
        // Créer une GRANDE grille pour tester les performances
        const int GRID_SIZE = 5; // 25 objets !
        const float SPACING = 3.0f;

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
                float posY = std::sin(x * 0.5f) * std::cos(z * 0.5f) * 2.0f;
                cube->setPosition(glm::vec3(posX, posY, posZ));

                cube->setRotation(glm::vec3(x * 5.0f, z * 5.0f, (x + z) * 3.0f));

                float scale = 0.6f + (std::sin(x * 0.3f + z * 0.3f) * 0.3f);
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

            // Calcul du FPS
            m_frameCount++;
            float timeSinceFPS = std::chrono::duration<float, std::chrono::seconds::period>(
                                     currentTime - lastFPSTime)
                                     .count();

            if (timeSinceFPS >= 1.0f)
            {
                m_fps = m_frameCount / timeSinceFPS;
                m_frameTime = timeSinceFPS / m_frameCount * 1000.0f;

                // Afficher les stats
                const auto &stats = m_commandRecorder->getStats();

                std::cout << "\r"
                          << "FPS:  " << std::setw(6) << std::fixed << std::setprecision(1) << m_fps
                          << " | Frame:  " << std::setw(5) << std::setprecision(2) << m_frameTime << "ms"
                          << " | Record: " << std::setw(5) << std::setprecision(2) << stats.lastRecordTime << "ms"
                          << " | Objects: " << std::setw(4) << stats.objectsRendered
                          << " | Threads: " << stats.threadsUsed
                          << " | Pending: " << std::setw(2) << m_threadPool->getPendingTasks()
                          << std::flush;

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

        delete m_renderer;
        delete m_scene;
        delete m_descriptorPool;
        delete m_material;
        delete m_uniformBuffer;
        delete m_commandRecorder; // Advanced Command Recorder
        delete m_pipeline;
        delete m_descriptorSetLayout;
        delete m_renderPass;
        delete m_swapChain;
        delete m_msaaBuffer;
        delete m_depthBuffer;

        vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);

        delete m_camera;

        // IMPORTANT :  Détruire le thread pool AVANT le device
        if (m_threadPool)
        {
            delete m_threadPool;
        }

        delete m_device;
        delete m_instance;
        delete m_window;

        printHeader("CLEANUP COMPLETED");
    }

    // Helpers d'affichage
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
        std::cout << "║  Objects in scene:      " << std::setw(28) << std::right << m_scene->getObjectCount() << "  ║" << std::endl;
        std::cout << "║  Worker threads:       " << std::setw(28) << m_threadPool->getThreadCount() << "  ║" << std::endl;
        std::cout << "║  MSAA samples:         " << std::setw(28) << m_msaaSamples << "  ║" << std::endl;
        std::cout << "║  Frames in flight:     " << std::setw(28) << MAX_FRAMES_IN_FLIGHT << "  ║" << std::endl;
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