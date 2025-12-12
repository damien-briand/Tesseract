#include "ThreadPool.hpp"
#include <iostream>
#include <algorithm>

ThreadPool::ThreadPool(size_t numThreads)
    : m_stop(false)
    , m_activeTasks(0)
{
    // Auto-detect du nombre de threads
    if (numThreads == 0) {
        numThreads = std::max(1u, std::thread::hardware_concurrency());
    }

    // Créer les threads workers
    m_threads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        m_threads.emplace_back(&ThreadPool::workerThread, this);
    }

    std::cout << "✓ Thread Pool created with " << numThreads << " threads" << std::endl;
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();

    for (std::thread& worker : m_threads) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "✓ Thread Pool destroyed" << std:: endl;
}

void ThreadPool::parallelFor(const std::vector<std::function<void()>>& tasks) {
    std::vector<std::future<void>> futures;
    futures.reserve(tasks.size());

    // Soumettre toutes les tâches
    for (const auto& task : tasks) {
        futures.push_back(enqueue(task));
    }

    // Attendre que toutes se terminent
    for (auto& future : futures) {
        future.get();
    }
}

void ThreadPool::waitAll() {
    // Attendre que toutes les tâches soient terminées
    while (m_activeTasks. load() > 0 || getPendingTasks() > 0) {
        std::this_thread::yield();
    }
}

size_t ThreadPool::getPendingTasks() const {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    return m_tasks.size();
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            m_condition.wait(lock, [this] {
                return m_stop || ! m_tasks.empty();
            });

            if (m_stop && m_tasks.empty()) {
                return;
            }

            if (!m_tasks.empty()) {
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
        }

        if (task) {
            m_activeTasks. fetch_add(1);
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "✗ Task failed: " << e.what() << std::endl;
            }
            m_activeTasks.fetch_sub(1);
        }
    }
}