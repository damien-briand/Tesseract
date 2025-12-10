#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class Device;
class RenderPass;
class Pipeline;
class SwapChain;
class DescriptorPool;
class Scene;

/**
 * @brief Gère l'enregistrement multi-threadé des command buffers
 * 
 * Crée un command pool et un command buffer secondaire par thread. 
 * Les threads enregistrent des portions de la scène en parallèle,
 * puis un command buffer primaire les exécute tous. 
 */
class ThreadedCommandBuffer {
public:
    /**
     * @brief Constructeur
     * @param device Device Vulkan
     * @param threadCount Nombre de threads de rendu (défaut:  4)
     * @param framesInFlight Nombre de frames en vol
     */
    ThreadedCommandBuffer(Device* device, uint32_t threadCount, uint32_t framesInFlight);
    ~ThreadedCommandBuffer();

    // Suppression de la copie
    ThreadedCommandBuffer(const ThreadedCommandBuffer&) = delete;
    ThreadedCommandBuffer& operator=(const ThreadedCommandBuffer&) = delete;

    /**
     * @brief Enregistre les command buffers en parallèle
     * @param frameIndex Index de la frame courante
     * @param imageIndex Index de l'image swapchain
     * @param renderPass Render pass
     * @param pipeline Pipeline graphique
     * @param swapChain Swap chain
     * @param descriptorPool Descriptor pool
     * @param scene Scène à rendre
     */
    void record(uint32_t frameIndex, uint32_t imageIndex, RenderPass* renderPass,
                Pipeline* pipeline, SwapChain* swapChain, DescriptorPool* descriptorPool,
                Scene* scene);

    /**
     * @brief Retourne le command buffer primaire pour une frame
     */
    VkCommandBuffer getPrimaryCommandBuffer(uint32_t frameIndex) const {
        return m_primaryCommandBuffers[frameIndex];
    }

    /**
     * @brief Retourne le command pool primaire
     */
    VkCommandPool getPrimaryCommandPool() const { return m_primaryCommandPool; }

private:
    Device* m_device;
    uint32_t m_threadCount;
    uint32_t m_framesInFlight;

    // Command pool et buffers PRIMAIRES (un par frame)
    VkCommandPool m_primaryCommandPool;
    std::vector<VkCommandBuffer> m_primaryCommandBuffers;

    // Command pools SECONDAIRES (un par thread)
    std::vector<VkCommandPool> m_secondaryCommandPools;
    
    // Command buffers SECONDAIRES (threadCount x framesInFlight)
    // Organisés comme :  [frame0_thread0, frame0_thread1, .. ., frame1_thread0, ...]
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;

    // Structures pour le multithreading
    struct ThreadData {
        uint32_t threadIndex;
        uint32_t frameIndex;
        uint32_t objectStart;
        uint32_t objectCount;
        VkCommandBuffer commandBuffer;
        RenderPass* renderPass;
        Pipeline* pipeline;
        SwapChain* swapChain;
        DescriptorPool* descriptorPool;
        Scene* scene;
    };

    // Méthodes privées
    void createCommandPools();
    void createCommandBuffers();
    void recordSecondaryCommandBuffer(const ThreadData& data);
    
    // Helper pour obtenir l'index du command buffer secondaire
    uint32_t getSecondaryBufferIndex(uint32_t frameIndex, uint32_t threadIndex) const {
        return frameIndex * m_threadCount + threadIndex;
    }
};