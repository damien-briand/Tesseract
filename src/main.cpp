#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
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

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const std::string SHADERS_PATH = "../src/shaders/spv/";
const std::string TEXTURE_PATH = "../assets/textures/texture.jpg";
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Données de test : un carré avec coordonnées de texture
const std::vector<Vertex> vertices = {
    // Position          Couleur                  TexCoord (UV)
    {{-0.5f, -0.5f},    {1.0f, 1.0f, 1.0f},    {0.0f, 1.0f}},  // Bas-gauche → Bas de texture (V=1.0)
    {{ 0.5f, -0.5f},    {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f}},  // Bas-droit → Bas de texture (V=1.0)
    {{ 0.5f,  0.5f},    {1.0f, 1.0f, 1.0f},    {1.0f, 0.0f}},  // Haut-droit → Haut de texture (V=0.0)
    {{-0.5f,  0.5f},    {1.0f, 1.0f, 1.0f},    {0.0f, 0.0f}}   // Haut-gauche → Haut de texture (V=0.0)
};

const std::vector<uint16_t> indices = {
    0, 1, 2,  // Premier triangle
    2, 3, 0   // Deuxième triangle
};

class Application {
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // Core objects
    Window* m_window = nullptr;
    Instance* m_instance = nullptr;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    Device* m_device = nullptr;
    SwapChain* m_swapChain = nullptr;

    // Rendering objects
    RenderPass* m_renderPass = nullptr;
    DescriptorSetLayout* m_descriptorSetLayout = nullptr;
    Pipeline* m_pipeline = nullptr;
    CommandBuffer* m_commandBuffer = nullptr;
    VertexBuffer* m_vertexBuffer = nullptr;
    IndexBuffer* m_indexBuffer = nullptr;
    UniformBuffer* m_uniformBuffer = nullptr;
    Texture* m_texture = nullptr;  // 🆕 Pointeur vers la texture
    DescriptorPool* m_descriptorPool = nullptr;
    Renderer* m_renderer = nullptr;

    void initVulkan() {
        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Tesseract - Texture Mapping");

        // 2. Créer l'instance Vulkan
        m_instance = new Instance("Tesseract", enableValidationLayers);

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());

        // 4.  Créer le device (physical + logical)
        m_device = new Device(m_instance->getHandle(), m_surface);

        // 5. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);

        // 6. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat());

        // 7.  Créer les framebuffers
        m_swapChain->createFramebuffers(m_renderPass->getHandle());

        // 8.  Créer le descriptor set layout (UBO + Texture Sampler)
        m_descriptorSetLayout = new DescriptorSetLayout(m_device);

        // 9. Créer le pipeline graphique (avec descriptor set layout)
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_descriptorSetLayout,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv"
        );

        // 10. Créer les command buffers
        m_commandBuffer = new CommandBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 11.  Créer le vertex buffer
        m_vertexBuffer = new VertexBuffer(m_device, vertices);

        // 12. Créer l'index buffer
        m_indexBuffer = new IndexBuffer(m_device, indices);

        // 13. Créer les uniform buffers
        m_uniformBuffer = new UniformBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 🆕 14. Charger la texture
        m_texture = new Texture(m_device, TEXTURE_PATH);

        // 🆕 15. Créer le descriptor pool avec texture (MODIFIÉ)
        m_descriptorPool = new DescriptorPool(
            m_device, 
            m_descriptorSetLayout, 
            m_uniformBuffer,
            m_texture,  // 🆕 Passer la texture
            MAX_FRAMES_IN_FLIGHT
        );

        // 16.  Créer le renderer (gère la synchronisation et le rendu)
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
            MAX_FRAMES_IN_FLIGHT
        );

        std::cout << "✓ Vulkan initialized successfully with texture support!" << std::endl;
    }

    void mainLoop() {
        while (!m_window->shouldClose()) {
            m_window->pollEvents();
            m_renderer->drawFrame();
        }

        m_renderer->waitIdle();
    }

    void cleanup() {
        // L'ordre de destruction est important ! 
        // On détruit dans l'ordre inverse de la création

        delete m_renderer;           // 16. Renderer (sync objects)
        delete m_descriptorPool;     // 15.  Descriptor pool
        delete m_texture;            // 🆕 14. Texture
        delete m_uniformBuffer;      // 13. Uniform buffers
        delete m_indexBuffer;        // 12. Index buffer
        delete m_vertexBuffer;       // 11.  Vertex buffer
        delete m_commandBuffer;      // 10. Command buffers
        delete m_pipeline;           // 9.   Pipeline
        delete m_descriptorSetLayout;// 8.   Descriptor set layout
        delete m_renderPass;         // 7.  Render pass
        delete m_swapChain;          // 6.  SwapChain
        
        // Surface (avant device car device l'utilise)
        vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);
        
        delete m_device;             // 5.  Device
        delete m_instance;           // 4.  Instance
        delete m_window;             // 3.  Window

        std::cout << "✓ Cleanup completed!" << std::endl;
    }
};

int main() {
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e. what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}