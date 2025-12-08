#include "Camera.hpp"
#include "Window.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

// ============================================================================
// CONSTRUCTEUR
// ============================================================================

Camera::Camera(const glm::vec3& position, float aspectRatio, float fov, float nearPlane, float farPlane)
    : m_position(position)
    , m_worldUp(0.0f, 0.0f, 1.0f)  // Z+ est "haut" dans votre projet
    , m_yaw(-90.0f)                // Commence en regardant vers -X
    , m_pitch(0.0f)
    , m_fov(fov)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane)
    , m_movementSpeed(2.5f)
    , m_sprintMultiplier(2.0f)
    , m_mouseSensitivity(0.1f)
    , m_enabled(true)
    , m_firstMouse(true)
    , m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
{
    updateCameraVectors();
}

// ============================================================================
// UPDATE - Appelée chaque frame
// ============================================================================

void Camera::update(Window* window, float deltaTime) {
    if (!m_enabled) {
        return;
    }

    // Capturer la souris dans la fenêtre
    glfwSetInputMode(window->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    processKeyboard(window, deltaTime);
    processMouse(window, deltaTime);
}

// ============================================================================
// GESTION DU CLAVIER
// ============================================================================

void Camera::processKeyboard(Window* window, float deltaTime) {
    GLFWwindow* glfwWindow = window->getHandle();

    // Calculer la vitesse effective (avec sprint si Shift pressé)
    float velocity = m_movementSpeed * deltaTime;
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        velocity *= m_sprintMultiplier;
    }

    // W : Avancer
    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
        m_position += m_front * velocity;
    }

    // S : Reculer
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
        m_position -= m_front * velocity;
    }

    // A : Gauche (strafe)
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS) {
        m_position -= m_right * velocity;
    }

    // D : Droite (strafe)
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS) {
        m_position += m_right * velocity;
    }

    // Q : Descendre
    if (glfwGetKey(glfwWindow, GLFW_KEY_Q) == GLFW_PRESS) {
        m_position -= m_worldUp * velocity;
    }

    // E : Monter
    if (glfwGetKey(glfwWindow, GLFW_KEY_E) == GLFW_PRESS) {
        m_position += m_worldUp * velocity;
    }

    // ESC : Désactiver la caméra (libérer la souris)
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_enabled = false;
        glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_firstMouse = true;  // Reset pour éviter un saut
    }

    // Clic gauche : Réactiver la caméra
    if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        m_enabled = true;
        m_firstMouse = true;
    }
}

// ============================================================================
// GESTION DE LA SOURIS
// ============================================================================

void Camera::processMouse(Window* window, float deltaTime) {
    // Récupérer la position actuelle de la souris
    double xpos, ypos;
    glfwGetCursorPos(window->getHandle(), &xpos, &ypos);

    // Premier mouvement de souris : initialiser sans faire bouger la caméra
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
        return;
    }

    // Calculer le déplacement
    double xoffset = xpos - m_lastMouseX;
    double yoffset = m_lastMouseY - ypos;  // Inversé car Y va de bas en haut
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    // Appliquer la sensibilité
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    // Mettre à jour les angles
    m_yaw += static_cast<float>(xoffset);
    m_pitch += static_cast<float>(yoffset);

    // Contraindre le pitch pour éviter le gimbal lock
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    // Mettre à jour les vecteurs de direction
    updateCameraVectors();
}

// ============================================================================
// MISE À JOUR DES VECTEURS DE DIRECTION
// ============================================================================

void Camera::updateCameraVectors() {
    // Calculer le nouveau vecteur "front" depuis yaw et pitch
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    // Recalculer les vecteurs "right" et "up"
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

// ============================================================================
// MATRICES
// ============================================================================

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    glm::mat4 proj = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    
    // IMPORTANT : Inverser l'axe Y pour Vulkan
    proj[1][1] *= -1;
    
    return proj;
}

// ============================================================================
// SETTERS
// ============================================================================

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
}

void Camera::setFOV(float fov) {
    m_fov = std::clamp(fov, 1.0f, 120.0f);
}

void Camera::updateAspectRatio(float width, float height) {
    m_aspectRatio = width / height;
}