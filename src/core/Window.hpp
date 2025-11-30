#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <functional>

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    // Suppression de la copie
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Getters
    GLFWwindow* getHandle() const { return m_window; }
    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    bool shouldClose() const;
    bool wasResized() const { return m_framebufferResized; }
    void resetResizedFlag() { m_framebufferResized = false; }

    // Méthodes
    void pollEvents();
    VkSurfaceKHR createSurface(VkInstance instance);
    void getFramebufferSize(int* width, int* height);

private:
    GLFWwindow* m_window;
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
    bool m_framebufferResized;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};