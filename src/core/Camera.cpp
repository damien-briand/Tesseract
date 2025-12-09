#include "Camera.hpp"
#include "Window.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

// ============================================================================
// CONSTRUCTEUR
// ============================================================================

Camera::Camera(const glm::vec3 &target, float distance, float aspectRatio)
    : m_target(target), m_distance(distance), m_yaw(-90.0f) // Regarde vers -X par défaut
      ,
      m_pitch(20.0f) // Légèrement de haut
      ,
      m_worldUp(0.0f, 0.0f, 1.0f) // Z+ est "haut"
      ,
      m_projectionMode(ProjectionMode::Perspective), m_fov(45.0f), m_orthoSize(5.0f), m_aspectRatio(aspectRatio), m_nearPlane(0.1f), m_farPlane(100.0f), m_rotationSpeed(0.3f), m_panSpeed(0.005f), m_zoomSpeed(0.5f), m_minDistance(0.5f), m_maxDistance(50.0f), m_isRotating(false), m_isPanning(false), m_lastMouseX(0.0), m_lastMouseY(0.0), m_firstMouse(true), m_keyPressCooldown(0.2f), m_timeSinceLastKeyPress(0.0f)
{
    updateCameraVectors();
}

// ============================================================================
// UPDATE
// ============================================================================

void Camera::update(Window *window, float deltaTime)
{
    m_timeSinceLastKeyPress += deltaTime;

    processMouseRotation(window, deltaTime);
    processMousePan(window, deltaTime);
    processMouseZoom(window, deltaTime);
    processKeyboardShortcuts(window);

    updateCameraVectors();
}

// ============================================================================
// ROTATION (Molette enfoncée)
// ============================================================================

void Camera::processMouseRotation(Window *window, float deltaTime)
{
    GLFWwindow *glfwWindow = window->getHandle();

    // Vérifier si la molette (bouton du milieu) est enfoncée
    bool middleButtonPressed = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    bool shiftPressed = glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    // Rotation = MMB sans Shift
    if (middleButtonPressed && !shiftPressed)
    {
        if (!m_isRotating)
        {
            m_isRotating = true;
            glfwGetCursorPos(glfwWindow, &m_lastMouseX, &m_lastMouseY);
            m_firstMouse = true;
        }

        double xpos, ypos;
        glfwGetCursorPos(glfwWindow, &xpos, &ypos);

        if (!m_firstMouse)
        {
            double xoffset = xpos - m_lastMouseX;
            double yoffset = m_lastMouseY - ypos; // Inversé

            m_yaw -= static_cast<float>(xoffset) * m_rotationSpeed;
            m_pitch += static_cast<float>(yoffset) * m_rotationSpeed;

            // Contraindre le pitch
            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        }

        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
    }
    else
    {
        m_isRotating = false;
        m_firstMouse = true;
    }
}

// ============================================================================
// PAN / TRANSLATION (Shift + Molette enfoncée)
// ============================================================================

void Camera::processMousePan(Window *window, float deltaTime)
{
    GLFWwindow *glfwWindow = window->getHandle();

    bool middleButtonPressed = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    bool shiftPressed = glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    // Pan = MMB avec Shift
    if (middleButtonPressed && shiftPressed)
    {
        if (!m_isPanning)
        {
            m_isPanning = true;
            glfwGetCursorPos(glfwWindow, &m_lastMouseX, &m_lastMouseY);
            m_firstMouse = true;
        }

        double xpos, ypos;
        glfwGetCursorPos(glfwWindow, &xpos, &ypos);

        if (!m_firstMouse)
        {
            double xoffset = xpos - m_lastMouseX;
            double yoffset = m_lastMouseY - ypos;

            // Déplacer le pivot dans le plan perpendiculaire à la vue
            float panSpeed = m_panSpeed * m_distance; // Plus on est loin, plus on se déplace vite
            m_target -= m_right * static_cast<float>(xoffset) * panSpeed;
            m_target += m_up * static_cast<float>(yoffset) * panSpeed;
        }

        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
    }
    else
    {
        m_isPanning = false;
        if (!m_isRotating)
        {
            m_firstMouse = true;
        }
    }
}

// ============================================================================
// ZOOM (Molette scroll)
// ============================================================================

void Camera::processMouseZoom(Window *window, float deltaTime)
{
    double scrollOffset = window->getScrollOffsetY();

    if (scrollOffset != 0.0)
    {
        // Zoom in/out en modifiant la distance
        m_distance -= static_cast<float>(scrollOffset) * m_zoomSpeed;
        m_distance = std::clamp(m_distance, m_minDistance, m_maxDistance);

        // Reset le scroll
        window->resetScrollOffset();
    }
}

// ============================================================================
// SHORTCUTS CLAVIER (Numpad)
// ============================================================================

void Camera::processKeyboardShortcuts(Window *window)
{
    GLFWwindow *glfwWindow = window->getHandle();

    // Cooldown pour éviter le spam
    if (m_timeSinceLastKeyPress < m_keyPressCooldown)
    {
        return;
    }

    // Vues prédéfinies
    if (glfwGetKey(glfwWindow, GLFW_KEY_KP_1) == GLFW_PRESS)
    {
        setViewFront();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_3) == GLFW_PRESS)
    {
        setViewRight();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_7) == GLFW_PRESS)
    {
        setViewTop();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_9) == GLFW_PRESS)
    {
        flipView();
        m_timeSinceLastKeyPress = 0.0f;
    }

    // Rotations discrètes
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_8) == GLFW_PRESS)
    {
        rotateUp();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_2) == GLFW_PRESS)
    {
        rotateDown();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_4) == GLFW_PRESS)
    {
        rotateLeft();
        m_timeSinceLastKeyPress = 0.0f;
    }
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_6) == GLFW_PRESS)
    {
        rotateRight();
        m_timeSinceLastKeyPress = 0.0f;
    }

    // Toggle Ortho/Perspective
    else if (glfwGetKey(glfwWindow, GLFW_KEY_KP_5) == GLFW_PRESS)
    {
        toggleProjectionMode();
        m_timeSinceLastKeyPress = 0.0f;
    }
}

// ============================================================================
// MISE À JOUR DES VECTEURS
// ============================================================================

void Camera::updateCameraVectors()
{
    // Calculer la position de la caméra en coordonnées sphériques
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    // Position relative au pivot
    glm::vec3 offset;
    offset.x = m_distance * cos(pitchRad) * cos(yawRad);
    offset.y = m_distance * cos(pitchRad) * sin(yawRad);
    offset.z = m_distance * sin(pitchRad);

    m_position = m_target + offset;

    // Vecteurs de direction
    m_front = glm::normalize(m_target - m_position);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

// ============================================================================
// MATRICES
// ============================================================================

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    glm::mat4 proj;

    if (m_projectionMode == ProjectionMode::Perspective)
    {
        proj = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    }
    else
    {
        // Orthographic
        float halfWidth = m_orthoSize * m_aspectRatio;
        float halfHeight = m_orthoSize;
        proj = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_nearPlane, m_farPlane);
    }

    // Inverser Y pour Vulkan
    proj[1][1] *= -1;

    return proj;
}

glm::vec3 Camera::getPosition() const
{
    return m_position;
}

// ============================================================================
// SETTERS
// ============================================================================

void Camera::setTarget(const glm::vec3 &target)
{
    m_target = target;
}

void Camera::setDistance(float distance)
{
    m_distance = std::clamp(distance, m_minDistance, m_maxDistance);
}

void Camera::toggleProjectionMode()
{
    if (m_projectionMode == ProjectionMode::Perspective)
    {
        m_projectionMode = ProjectionMode::Orthographic;
        std::cout << "Switched to Orthographic view" << std::endl;
    }
    else
    {
        m_projectionMode = ProjectionMode::Perspective;
        std::cout << "Switched to Perspective view" << std::endl;
    }
}

void Camera::updateAspectRatio(float width, float height)
{
    m_aspectRatio = width / height;
}

// ============================================================================
// VUES PRÉDÉFINIES
// ============================================================================

void Camera::setViewFront()
{
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    std::cout << "Front View (Numpad 1)" << std::endl;
}

void Camera::setViewRight()
{
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    std::cout << "Right View (Numpad 3)" << std::endl;
}

void Camera::setViewTop()
{
    m_yaw = -90.0f;
    m_pitch = 89.0f; // Presque vertical pour éviter gimbal lock
    std::cout << "Top View (Numpad 7)" << std::endl;
}

void Camera::setViewBack()
{
    m_yaw = 90.0f;
    m_pitch = 0.0f;
}

void Camera::setViewLeft()
{
    m_yaw = 180.0f;
    m_pitch = 0.0f;
}

void Camera::setViewBottom()
{
    m_yaw = -90.0f;
    m_pitch = -89.0f;
}

void Camera::flipView()
{
    m_yaw += 180.0f;
    if (m_yaw > 180.0f)
        m_yaw -= 360.0f;
    std::cout << "Flipped View (Numpad 9)" << std::endl;
}

// ============================================================================
// ROTATIONS DISCRÈTES (15° par pas)
// ============================================================================

void Camera::rotateUp()
{
    m_pitch += 15.0f;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::rotateDown()
{
    m_pitch -= 15.0f;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::rotateLeft()
{
    m_yaw -= 15.0f;
}

void Camera::rotateRight()
{
    m_yaw += 15.0f;
}