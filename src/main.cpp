#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "core/Camera.hpp"

#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "rendering/ThreadedCommandBuffer.hpp"
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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
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

    // Rendering objects
    RenderPass *m_renderPass = nullptr;
    DescriptorSetLayout *m_descriptorSetLayout = nullptr;
    Pipeline *m_pipeline = nullptr;
    ThreadedCommandBuffer *m_commandBuffer = nullptr;
    UniformBuffer *m_uniformBuffer = nullptr;
    Material *m_material = nullptr;
    DepthBuffer *m_depthBuffer = nullptr;
    DescriptorPool *m_descriptorPool = nullptr;
    Renderer *m_renderer = nullptr;

    MSAABuffer *m_msaaBuffer = nullptr;
    VkSampleCountFlagBits m_msaaSamples;

    // NOUVEAU : Scene au lieu de VertexBuffer/IndexBuffer
    Scene *m_scene = nullptr;

    void initVulkan()
    {
        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Tesseract - Multiple Objects");

        // 2. Créer l'instance Vulkan
        m_instance = new Instance("Tesseract", enableValidationLayers);

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());

        // 4. Créer la caméra
        float aspectRatio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
        m_camera = new Camera(
            glm::vec3(0.0f, 0.0f, 0.0f), // Pivot au centre
            8.0f,                        // Distance initiale (plus loin pour voir plusieurs objets)
            aspectRatio);

        // 5. Créer le device (physical + logical)
        m_device = new Device(m_instance->getHandle(), m_surface);

        // Déterminer le nombre de samples MSAA supportés
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        // 6. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);

        // 7. Créer le buffer MSAA
        m_msaaBuffer = new MSAABuffer(m_device, m_swapChain, m_msaaSamples);

        // 8. Créer le depth buffer
        m_depthBuffer = new DepthBuffer(m_device, m_swapChain, m_msaaSamples);

        // 9. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat(),
                                      m_depthBuffer->getFormat(), m_msaaSamples);

        // 10. Créer les framebuffers
        m_swapChain->createFramebuffers(m_renderPass->getHandle(),
                                        m_msaaBuffer->getImageView(),
                                        m_depthBuffer->getImageView());

        // 11. Créer le descriptor set layout
        m_descriptorSetLayout = new DescriptorSetLayout(m_device);

        // 12. Créer le pipeline graphique
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_descriptorSetLayout,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv",
            m_msaaSamples);

        // 13. Créer les command buffers
        m_commandBuffer = new ThreadedCommandBuffer(m_device, 4, MAX_FRAMES_IN_FLIGHT);

        // 14. Créer les uniform buffers
        m_uniformBuffer = new UniformBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 15. Charger le material
        m_material = new Material(m_device,
                                  "../assets/models/Cube/Color.png",
                                  "../assets/models/Cube/Normal.png",
                                  "../assets/models/Cube/Metalic.png",
                                  "");

        // 16. Créer le descriptor pool
        m_descriptorPool = new DescriptorPool(
            m_device,
            m_descriptorSetLayout,
            m_uniformBuffer,
            m_material,
            MAX_FRAMES_IN_FLIGHT);

        m_scene = new Scene(m_device);

        createMultipleObjects();

        // 17. Créer le renderer (MODIFIÉ : sans VertexBuffer/IndexBuffer, avec Scene)
        m_renderer = new Renderer(
            m_device,
            m_swapChain,
            m_window,
            m_renderPass,
            m_pipeline,
            m_commandBuffer,
            m_uniformBuffer,
            m_descriptorPool,
            m_msaaBuffer,
            m_depthBuffer,
            MAX_FRAMES_IN_FLIGHT,
            m_camera,
            m_scene);

        std::cout << "✓ Vulkan initialized successfully with "
                  << m_scene->getObjectCount() << " objects!" << std::endl;
    }

    // ========================================================================
    // Fonction pour créer plusieurs objets
    // ========================================================================
    void createMultipleObjects()
    {
        // Objet 1 : Cube au centre
        GameObject *cube1 = m_scene->createGameObject("Cube_Center");
        cube1->loadMeshFromFile("../assets/models/Cube/Cube.obj");
        cube1->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        cube1->setMaterial(m_material);

        // Objet 2 : Cube à gauche
        GameObject *cube2 = m_scene->createGameObject("Cube_Left");
        cube2->loadMeshFromFile("../assets/models/Cube/Cube.obj");
        cube2->setPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
        cube2->setRotation(glm::vec3(0.0f, 0.0f, 45.0f));
        cube2->setMaterial(m_material);

        // Objet 3 : Cube à droite
        GameObject *cube3 = m_scene->createGameObject("Cube_Right");
        cube3->loadMeshFromFile("../assets/models/Cube/Cube.obj");
        cube3->setPosition(glm::vec3(2.5f, 0.0f, 0.0f));
        cube3->setRotation(glm::vec3(45.0f, 0.0f, 0.0f));
        cube3->setUniformScale(0.7f);
        cube3->setMaterial(m_material);

        // Objet 4 : Cube en haut
        GameObject *cube4 = m_scene->createGameObject("Cube_Top");
        cube4->loadMeshFromFile("../assets/models/Cube/Cube.obj");
        cube4->setPosition(glm::vec3(0.0f, 2.5f, 0.0f));
        cube4->setRotation(glm::vec3(0.0f, 45.0f, 0.0f));
        cube4->setUniformScale(1.2f);
        cube4->setMaterial(m_material);

        // Objet 5 : Cube en bas
        GameObject *cube5 = m_scene->createGameObject("Cube_Bottom");
        cube5->loadMeshFromFile("../assets/models/Cube/Cube.obj");
        cube5->setPosition(glm::vec3(0.0f, -2.5f, 0.0f));
        cube5->setRotation(glm::vec3(30.0f, 30.0f, 30.0f));
        cube5->setUniformScale(0.5f);
        cube5->setMaterial(m_material);

        std::cout << "✓ Created " << m_scene->getObjectCount() << " game objects" << std::endl;
    }

    void mainLoop()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();

        while (!m_window->shouldClose())
        {
            // Calculer deltaTime
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(
                                  currentTime - lastTime)
                                  .count();
            lastTime = currentTime;

            m_window->pollEvents();
            m_camera->update(m_window, deltaTime);
            m_renderer->drawFrame();
        }

        m_renderer->waitIdle();
    }

    void cleanup()
    {
        // L'ordre de destruction est important !
        delete m_renderer;            // 17. Renderer
        delete m_scene;               // NOUVEAU : Scene (contient les GameObjects)
        delete m_descriptorPool;      // 16. Descriptor pool
        delete m_material;            // 15. Material
        delete m_uniformBuffer;       // 14. Uniform buffers
        delete m_commandBuffer;       // 13. Command buffers
        delete m_pipeline;            // 12. Pipeline
        delete m_descriptorSetLayout; // 11. Descriptor set layout
        delete m_renderPass;          // 10. Render pass
        delete m_swapChain;           // 9. SwapChain
        delete m_msaaBuffer;          // 8. MSAA buffer
        delete m_depthBuffer;         // 7. Depth buffer

        // Surface (avant device car device l'utilise)
        vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);

        delete m_camera;   // 6. Caméra
        delete m_device;   // 5. Device
        delete m_instance; // 4. Instance
        delete m_window;   // 3. Window

        std::cout << "✓ Cleanup completed!" << std::endl;
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
        std::cerr << "✗ Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}