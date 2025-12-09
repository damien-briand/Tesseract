#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "core/Camera.hpp"

#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "rendering/CommandBuffer.hpp"
#include "rendering/Buffer/VertexBuffer.hpp"
#include "rendering/Buffer/IndexBuffer.hpp"
#include "rendering/Buffer/UniformBuffer.hpp"
#include "rendering/Descriptor/DescriptorSetLayout.hpp"
#include "rendering/Descriptor/DescriptorPool.hpp"
#include "rendering/Texture/Texture.hpp"
#include "rendering/Material/Material.hpp"
#include "rendering/DepthBuffer.hpp"
#include "rendering/MSAABuffer.hpp"

#include "utils/ModelLoader.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const std::string SHADERS_PATH = "../src/shaders/spv/";
const std::string TEXTURE_PATH = "../assets/models/viking_room.png";
const std::string MODEL_PATH = "../assets/models/Cube/Cube.obj";
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
    CommandBuffer *m_commandBuffer = nullptr;
    VertexBuffer *m_vertexBuffer = nullptr;
    IndexBuffer *m_indexBuffer = nullptr;
    UniformBuffer *m_uniformBuffer = nullptr;
    Material *m_material = nullptr;
    DepthBuffer *m_depthBuffer = nullptr;
    DescriptorPool *m_descriptorPool = nullptr;
    Renderer *m_renderer = nullptr;

    MSAABuffer *m_msaaBuffer = nullptr;
    VkSampleCountFlagBits m_msaaSamples;

    void initVulkan()
    {
        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Tesseract");

        // 2.  Créer l'instance Vulkan
        m_instance = new Instance("Tesseract", enableValidationLayers);

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());

        // Créer la caméra
        float aspectRatio = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
        m_camera = new Camera(
            glm::vec3(0.0f, 0.0f, 0.0f), // Pivot au centre
            5.0f,                        // Distance initiale
            static_cast<float>(WIDTH) / static_cast<float>(HEIGHT));

        // 4.  Créer le device (physical + logical)
        m_device = new Device(m_instance->getHandle(), m_surface);

        // Déterminer le nombre de samples MSAA supportés
        m_msaaSamples = m_device->getMaxUsableSampleCount();

        // 5. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);

        // Créer le buffer MSAA
        m_msaaBuffer = new MSAABuffer(m_device, m_swapChain, m_msaaSamples);

        // 6. Créer le depth buffer
        m_depthBuffer = new DepthBuffer(m_device, m_swapChain, m_msaaSamples);

        // 7. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat(), m_depthBuffer->getFormat(), m_msaaSamples);

        // 8. Créer les framebuffers (MODIFIÉ : avec depth buffer)
        m_swapChain->createFramebuffers(m_renderPass->getHandle(),
                                        m_msaaBuffer->getImageView(),
                                        m_depthBuffer->getImageView());

        // 9.  Créer le descriptor set layout (UBO + Texture Sampler)
        m_descriptorSetLayout = new DescriptorSetLayout(m_device);

        // 10. Créer le pipeline graphique (avec descriptor set layout)
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_descriptorSetLayout,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv", m_msaaSamples);

        // 11. Créer les command buffers
        m_commandBuffer = new CommandBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 11. Charger le modèle 3D
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        if (!ModelLoader::loadModel(MODEL_PATH, vertices, indices))
        {
            throw std::runtime_error("Failed to load model: " + MODEL_PATH);
        }

        // 12.  Créer le vertex buffer
        m_vertexBuffer = new VertexBuffer(m_device, vertices);

        // 13. Créer l'index buffer
        m_indexBuffer = new IndexBuffer(m_device, indices);

        // 14. Créer les uniform buffers
        m_uniformBuffer = new UniformBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 15. Charger la texture
        m_material = new Material(m_device,
                                  "../assets/models/Cube/Color.png",
                                  "../assets/models/Cube/Normal.png",
                                  "../assets/models/Cube/Metalic.png",
                                  "");

        // 16. Créer le descriptor pool avec texture
        m_descriptorPool = new DescriptorPool(
            m_device,
            m_descriptorSetLayout,
            m_uniformBuffer,
            m_material,
            MAX_FRAMES_IN_FLIGHT);

        // 17. Créer le renderer (gère la synchronisation et le rendu)
        m_renderer = new Renderer(
            m_device,
            m_swapChain,
            m_window,
            m_renderPass,
            m_pipeline,
            m_commandBuffer,
            m_vertexBuffer,
            m_indexBuffer,
            m_uniformBuffer,
            m_descriptorPool,
            m_msaaBuffer,
            m_depthBuffer,
            MAX_FRAMES_IN_FLIGHT,
            m_camera);

        std::cout << "✓ Vulkan initialized successfully with depth buffering!" << std::endl;
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
        // On détruit dans l'ordre inverse de la création

        delete m_renderer;            // 17. Renderer (sync objects)
        delete m_descriptorPool;      // 16.  Descriptor pool
        delete m_material;            // 15.  Material
        delete m_uniformBuffer;       // 14.  Uniform buffers
        delete m_indexBuffer;         // 13. Index buffer
        delete m_vertexBuffer;        // 12.  Vertex buffer
        delete m_commandBuffer;       // 11. Command buffers
        delete m_pipeline;            // 10. Pipeline
        delete m_descriptorSetLayout; // 9.   Descriptor set layout
        delete m_renderPass;          // 8.   Render pass
        delete m_swapChain;           // 7.  SwapChain
        delete m_msaaBuffer;          // MSAA buffer
        delete m_depthBuffer;         // 6. Depth buffer

        // Surface (avant device car device l'utilise)
        vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);

        delete m_camera;   // Caméra
        delete m_device;   // 5.   Device
        delete m_instance; // 4.  Instance
        delete m_window;   // 3.  Window

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