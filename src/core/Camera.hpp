#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Window;

/**
 * @brief Caméra FPS avec contrôles clavier (WASD + QE) et souris
 * 
 * Système de coordonnées :
 * - X : droite
 * - Y : haut
 * - Z : vers la caméra (right-handed, comme OpenGL, inversé ensuite pour Vulkan)
 * 
 * Contrôles :
 * - W/S : Avancer/Reculer
 * - A/D : Gauche/Droite (strafe)
 * - Q/E : Descendre/Monter
 * - Souris : Rotation (yaw/pitch)
 * - Shift : Accélérer
 */
class Camera {
public:
    /**
     * @brief Constructeur
     * @param position Position initiale de la caméra
     * @param aspectRatio Ratio largeur/hauteur (width/height)
     * @param fov Champ de vision en degrés (défaut: 45°)
     * @param nearPlane Plan proche (défaut: 0. 1)
     * @param farPlane Plan lointain (défaut: 100.0)
     */
    Camera(const glm::vec3& position = glm::vec3(2.0f, 2.0f, 2.0f),
           float aspectRatio = 800.0f / 600.0f,
           float fov = 45.0f,
           float nearPlane = 0.1f,
           float farPlane = 100.0f);

    ~Camera() = default;

    // ========================================================================
    // MISE À JOUR
    // ========================================================================
    
    /**
     * @brief Met à jour la caméra en fonction des inputs (appelé chaque frame)
     * @param window Pointeur vers la fenêtre GLFW (pour les inputs)
     * @param deltaTime Temps écoulé depuis la dernière frame (en secondes)
     */
    void update(Window* window, float deltaTime);

    /**
     * @brief Met à jour l'aspect ratio (lors d'un resize de fenêtre)
     * @param width Nouvelle largeur
     * @param height Nouvelle hauteur
     */
    void updateAspectRatio(float width, float height);

    // ========================================================================
    // GETTERS - Matrices
    // ========================================================================
    
    /**
     * @brief Retourne la matrice View (transformation monde → caméra)
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Retourne la matrice Projection
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * @brief Retourne la position de la caméra dans l'espace monde
     */
    const glm::vec3& getPosition() const { return m_position; }

    /**
     * @brief Retourne la direction "avant" de la caméra (normalisée)
     */
    const glm::vec3& getFront() const { return m_front; }

    /**
     * @brief Retourne la direction "droite" de la caméra (normalisée)
     */
    const glm::vec3& getRight() const { return m_right; }

    /**
     * @brief Retourne la direction "haut" de la caméra (normalisée)
     */
    const glm::vec3& getUp() const { return m_up; }

    // ========================================================================
    // SETTERS - Configuration
    // ========================================================================
    
    void setPosition(const glm::vec3& position);
    void setFOV(float fov);
    void setMovementSpeed(float speed) { m_movementSpeed = speed; }
    void setSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

    // ========================================================================
    // CONTRÔLES
    // ========================================================================
    
    /**
     * @brief Active/désactive le mode caméra libre (capture de la souris)
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

private:
    // ========================================================================
    // MÉTHODES PRIVÉES
    // ========================================================================
    
    /**
     * @brief Met à jour les vecteurs de direction (front, right, up) depuis yaw/pitch
     */
    void updateCameraVectors();

    /**
     * @brief Gère les inputs clavier (WASD, QE, Shift)
     */
    void processKeyboard(Window* window, float deltaTime);

    /**
     * @brief Gère les inputs souris (rotation)
     */
    void processMouse(Window* window, float deltaTime);

    // ========================================================================
    // PARAMÈTRES DE POSITION ET ORIENTATION
    // ========================================================================
    
    glm::vec3 m_position;      // Position dans l'espace monde
    glm::vec3 m_front;         // Direction "avant" (normalisée)
    glm::vec3 m_up;            // Direction "haut" (normalisée)
    glm::vec3 m_right;         // Direction "droite" (normalisée)
    glm::vec3 m_worldUp;       // Vecteur "haut" du monde (généralement Y+)

    // Angles d'Euler (en degrés)
    float m_yaw;               // Rotation autour de l'axe Y (gauche/droite)
    float m_pitch;             // Rotation autour de l'axe X (haut/bas)

    // ========================================================================
    // PARAMÈTRES DE PROJECTION
    // ========================================================================
    
    float m_fov;               // Field of View en degrés
    float m_aspectRatio;       // Ratio largeur/hauteur
    float m_nearPlane;         // Plan de clipping proche
    float m_farPlane;          // Plan de clipping lointain

    // ========================================================================
    // PARAMÈTRES DE CONTRÔLE
    // ========================================================================
    
    float m_movementSpeed;     // Vitesse de déplacement (unités/seconde)
    float m_sprintMultiplier;  // Multiplicateur quand Shift est pressé
    float m_mouseSensitivity;  // Sensibilité de la souris

    bool m_enabled;            // Si la caméra est active (capture souris)
    bool m_firstMouse;         // Pour éviter un saut au premier mouvement
    double m_lastMouseX;       // Dernière position X de la souris
    double m_lastMouseY;       // Dernière position Y de la souris
};