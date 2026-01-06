#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class Device;
class RenderPass;
class Pipeline;
class SwapChain;
class DescriptorPool;
class Scene;
class ThreadPool;
class ImGuiManager;

/**
 * @brief Enregistrement avancé de command buffers avec thread pool
 *
 * Features avancées :
 * - Thread pool persistant (pas de création/destruction par frame)
 * - Work stealing pour load balancing
 * - Statistiques de performance
 * - Support du culling frustum (prochaine étape)
 * - Support ImGui rendering
 */
class AdvancedCommandRecorder
{
public:
    AdvancedCommandRecorder(Device *device, ThreadPool *threadPool, uint32_t framesInFlight, Pipeline* gizmoPipeline);
    ~AdvancedCommandRecorder();

    AdvancedCommandRecorder(const AdvancedCommandRecorder &) = delete;
    AdvancedCommandRecorder &operator=(const AdvancedCommandRecorder &) = delete;

    /**
     * @brief Enregistre les command buffers en utilisant le thread pool
     * @param imguiManager (Optionnel) Manager ImGui pour le rendu GUI
     */
    void record(
        uint32_t frameIndex,
        uint32_t imageIndex,
        RenderPass *renderPass,
        Pipeline *pipeline,
        SwapChain *swapChain,
        DescriptorPool *descriptorPool,
        Scene *scene,
        ImGuiManager *imguiManager = nullptr);

    VkCommandBuffer getPrimaryCommandBuffer(uint32_t frameIndex) const
    {
        return m_primaryCommandBuffers[frameIndex];
    }

    // Statistiques
    struct Stats
    {
        float lastRecordTime = 0.0f; // Temps d'enregistrement en ms
        uint32_t objectsRendered = 0;
        uint32_t threadsUsed = 0;
    };

    const Stats &getStats() const { return m_stats; }

private:
    Device *m_device;
    ThreadPool *m_threadPool;
    uint32_t m_framesInFlight;
    uint32_t m_threadCount;

    Pipeline* m_gizmoPipeline; // Pipeline unlit pour le gizmo

    // Command pools et buffers
    VkCommandPool m_primaryCommandPool;
    std::vector<VkCommandBuffer> m_primaryCommandBuffers;
    std::vector<VkCommandPool> m_secondaryCommandPools;
    std::vector<VkCommandBuffer> m_secondaryCommandBuffers;
    
    // Command pool et buffers pour ImGui (un par frame)
    VkCommandPool m_imguiCommandPool;
    std::vector<VkCommandBuffer> m_imguiCommandBuffers;

    // Statistiques
    Stats m_stats;

    struct RenderTask
    {
        uint32_t threadIndex;
        uint32_t frameIndex;
        uint32_t objectStart;
        uint32_t objectCount;
        VkCommandBuffer commandBuffer;
        RenderPass *renderPass;
        Pipeline *pipeline;
        SwapChain *swapChain;
        DescriptorPool *descriptorPool;
        Scene *scene;
    };

    void createCommandPools();
    void createCommandBuffers();
    void recordSecondaryCommandBuffer(const RenderTask &task);

    uint32_t getSecondaryBufferIndex(uint32_t frameIndex, uint32_t threadIndex) const
    {
        return frameIndex * m_threadCount + threadIndex;
    }
};