#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/CommandBuffer.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "rendering/VertexBuffer.hpp"
#include "rendering/IndexBuffer.hpp"

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

// Données de test : un carré coloré avec indices
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // Coin haut-gauche - Rouge
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},   // Coin haut-droit - Vert
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    // Coin bas-droit - Bleu
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}    // Coin bas-gauche - Blanc
};

const std::vector<uint16_t> indices = {
    0, 1, 2,  // Premier triangle (haut-gauche, haut-droit, bas-droit)
    2, 3, 0   // Deuxième triangle (bas-droit, bas-gauche, haut-gauche)
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
    Pipeline* m_pipeline = nullptr;
    CommandBuffer* m_commandBuffer = nullptr;
    VertexBuffer* m_vertexBuffer = nullptr;
    IndexBuffer* m_indexBuffer = nullptr;
    Renderer* m_renderer = nullptr;

    void initVulkan() {
        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Vulkan Triangle");

        // 2. Créer l'instance Vulkan
        m_instance = new Instance("Hello Triangle", enableValidationLayers);

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());

        // 4. Créer le device (physical + logical)
        m_device = new Device(m_instance->getHandle(), m_surface);

        // 5. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);

        // 6. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat());

        // 7. Créer les framebuffers
        m_swapChain->createFramebuffers(m_renderPass->getHandle());

        // 8. Créer le pipeline graphique
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv"
        );

        // 9. Créer les command buffers
        m_commandBuffer = new CommandBuffer(m_device, MAX_FRAMES_IN_FLIGHT);

        // 10. Créer le vertex buffer
        m_vertexBuffer = new VertexBuffer(m_device, vertices);

        // 11. Créer l'index buffer
        m_indexBuffer = new IndexBuffer(m_device, indices);

        // 12. Créer le renderer (gère la synchronisation et le rendu)
        m_renderer = new Renderer(
            m_device,
            m_swapChain,
            m_window,
            m_renderPass,
            m_pipeline,
            m_commandBuffer,
            m_vertexBuffer,
            m_indexBuffer,
            MAX_FRAMES_IN_FLIGHT
        );

        std::cout << "✓ Vulkan initialized successfully!" << std::endl;
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

        delete m_renderer;           // 12. Renderer (sync objects)
        delete m_indexBuffer;        // 11. Index buffer
        delete m_vertexBuffer;       // 10. Vertex buffer
        delete m_commandBuffer;      // 9.  Command buffers
        delete m_pipeline;           // 8.  Pipeline
        delete m_renderPass;         // 7.  Render pass (après swapchain car framebuffers)
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
        std::cerr << "✗ Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}