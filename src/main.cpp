#include "core/Window.hpp"
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/SwapChain.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/Pipeline.hpp"
#include "rendering/CommandBuffer.hpp"
#include "rendering/Renderer.hpp"
#include "utils/VulkanUtils.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const std::string SHADERS_PATH = "../src/shaders/spv/";
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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
    Renderer* m_renderer = nullptr;

    void initVulkan() {
        // Afficher les informations Vulkan disponibles (optionnel)
        if (enableValidationLayers) {
            std::cout << "\n===== Vulkan Initialization =====" << std::endl;
            // VulkanUtils::printAvailableExtensions();
            // VulkanUtils::printAvailableLayers();
        }

        // 1. Créer la fenêtre
        m_window = new Window(WIDTH, HEIGHT, "Vulkan Triangle");
        std::cout << "✓ Window created" << std::endl;

        // 2. Créer l'instance Vulkan
        m_instance = new Instance("Hello Triangle", enableValidationLayers);
        std::cout << "✓ Vulkan instance created" << std::endl;

        // 3. Créer la surface
        m_surface = m_window->createSurface(m_instance->getHandle());
        std::cout << "✓ Surface created" << std::endl;

        // 4. Créer le device (physical + logical)
        m_device = new Device(m_instance->getHandle(), m_surface);
        std::cout << "✓ Device created" << std::endl;

        // Afficher les infos du GPU (optionnel)
        if (enableValidationLayers) {
            VulkanUtils::printPhysicalDeviceInfo(m_device->getPhysicalDevice());
        }

        // 5. Créer la swapchain
        m_swapChain = new SwapChain(m_device, m_surface, m_window);
        std::cout << "✓ SwapChain created (" << m_swapChain->getImageCount() << " images)" << std::endl;
        std::cout << "  Format: " << VulkanUtils::formatToString(m_swapChain->getImageFormat()) << std::endl;
        std::cout << "  Extent: " << m_swapChain->getExtent().width << "x" 
                  << m_swapChain->getExtent().height << std::endl;

        // 6. Créer le render pass
        m_renderPass = new RenderPass(m_device, m_swapChain->getImageFormat());
        std::cout << "✓ RenderPass created" << std::endl;

        // 7. Créer les framebuffers
        m_swapChain->createFramebuffers(m_renderPass->getHandle());
        std::cout << "✓ Framebuffers created" << std::endl;

        // 8. Créer le pipeline graphique
        m_pipeline = new Pipeline(
            m_device,
            m_renderPass,
            m_swapChain->getExtent(),
            SHADERS_PATH + "vert.spv",
            SHADERS_PATH + "frag.spv"
        );
        std::cout << "✓ Graphics pipeline created" << std::endl;

        // 9. Créer les command buffers
        m_commandBuffer = new CommandBuffer(m_device, MAX_FRAMES_IN_FLIGHT);
        std::cout << "✓ Command buffers created" << std::endl;

        // 10. Créer le renderer (gère la synchronisation et le rendu)
        m_renderer = new Renderer(
            m_device,
            m_swapChain,
            m_window,
            m_renderPass,
            m_pipeline,
            m_commandBuffer,
            MAX_FRAMES_IN_FLIGHT
        );
        std::cout << "✓ Renderer created" << std::endl;

        std::cout << "\n✓ Vulkan initialization complete!\n" << std::endl;
    }

    void mainLoop() {
        std::cout << "Entering main loop..." << std::endl;
        
        while (!m_window->shouldClose()) {
            m_window->pollEvents();
            m_renderer->drawFrame();
        }

        m_renderer->waitIdle();
        std::cout << "\nExited main loop" << std::endl;
    }

    void cleanup() {
        std::cout << "\nCleaning up resources..." << std::endl;

        // L'ordre de destruction est important !
        // On détruit dans l'ordre inverse de la création

        delete m_renderer;           // 10. Renderer (sync objects)
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
        std::cerr << "\n✗ Fatal Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}