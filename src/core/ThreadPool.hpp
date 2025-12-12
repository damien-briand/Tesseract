#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

/**
 * @brief Thread Pool avec work-stealing pour tâches asynchrones
 * 
 * Architecture professionnelle utilisée dans les moteurs de jeu modernes. 
 * Features :
 * - Pool de threads persistants
 * - File de tâches thread-safe
 * - Work stealing pour load balancing
 * - Support des futures pour récupérer des résultats
 */
class ThreadPool {
public:
    /**
     * @brief Constructeur
     * @param numThreads Nombre de threads (0 = auto-detect)
     */
    explicit ThreadPool(size_t numThreads = 0);
    ~ThreadPool();

    // Suppression de la copie
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Ajoute une tâche à la file
     * @tparam F Type de la fonction
     * @tparam Args Types des arguments
     * @return Future pour récupérer le résultat
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Execute plusieurs tâches en parallèle et attend qu'elles finissent
     * @param tasks Vecteur de tâches à exécuter
     */
    void parallelFor(const std::vector<std::function<void()>>& tasks);

    /**
     * @brief Attend que toutes les tâches en cours soient terminées
     */
    void waitAll();

    /**
     * @brief Retourne le nombre de threads
     */
    size_t getThreadCount() const { return m_threads.size(); }

    /**
     * @brief Retourne le nombre de tâches en attente
     */
    size_t getPendingTasks() const;

private:
    // Threads workers
    std::vector<std:: thread> m_threads;
    
    // File de tâches
    std::queue<std::function<void()>> m_tasks;
    
    // Synchronisation
    mutable std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop;
    std::atomic<size_t> m_activeTasks;

    // Fonction exécutée par chaque thread worker
    void workerThread();
};

// ============================================================================
// IMPLÉMENTATION DES TEMPLATES
// ============================================================================

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        // Ne pas permettre l'ajout de tâches après l'arrêt
        if (m_stop) {
            throw std::runtime_error("ThreadPool is stopped");
        }

        m_tasks.emplace([task](){ (*task)(); });
    }
    m_condition.notify_one();
    return res;
}