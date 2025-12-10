#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Window;

/**
 * @brief Caméra orbite style Blender avec contrôles souris
 * 
 * Contrôles :
 * - Molette enfoncée (MMB) : Rotation autour du pivot
 * - Shift + Molette enfoncée : Translation (pan)
 * - Scroll molette : Zoom
 * - Numpad 1/3/7 : Vues front/side/top
 * - Numpad 2/4/6/8 : Rotation par pas de 15°
 * - Numpad 9 : Vue opposée
 * - Numpad 5 : Toggle Ortho/Perspective
 */
class Camera {
public:
    enum class ProjectionMode {
        Perspective,
        Orthographic
    };

    /**
     * @brief Constructeur
     * @param target Point autour duquel la caméra orbite
     * @param distance Distance initiale du pivot
     * @param aspectRatio Ratio largeur/hauteur
     */
    Camera(const glm:: vec3& target = glm:: vec3(0.0f, 0.0f, 0.0f),
           float distance = 5.0f,
           float aspectRatio = 800.0f / 600.0f);

    ~Camera() = default;

    // ========================================================================
    // MISE À JOUR
    // ========================================================================
    
    /**
     * @brief Met à jour la caméra en fonction des inputs
     * @param window Pointeur vers la fenêtre GLFW
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    void update(Window* window, float deltaTime);

    /**
     * @brief Met à jour l'aspect ratio (lors d'un resize)
     */
    void updateAspectRatio(float width, float height);

    // ========================================================================
    // GETTERS - Matrices
    // ========================================================================
    
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 getPosition() const;
    const glm::vec3& getTarget() const { return m_target; }
    const glm::vec3& getFront() const { return m_front; }
    const glm:: vec3& getUp() const { return m_up; }
    const glm::vec3& getRight() const { return m_right; }

    // ========================================================================
    // SETTERS
    // ========================================================================
    
    void setTarget(const glm::vec3& target);
    void setDistance(float distance);
    void setProjectionMode(ProjectionMode mode) { m_projectionMode = mode; }
    void toggleProjectionMode();
    
    // Vitesses
    void setRotationSpeed(float speed) { m_rotationSpeed = speed; }
    void setPanSpeed(float speed) { m_panSpeed = speed; }
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }

    // ========================================================================
    // VUES PRÉDÉFINIES (Numpad)
    // ========================================================================
    
    void setViewFront();      // Numpad 1
    void setViewRight();      // Numpad 3
    void setViewTop();        // Numpad 7
    void setViewBack();       // Opposé de front
    void setViewLeft();       // Opposé de right
    void setViewBottom();     // Opposé de top
    void flipView();          // Numpad 9 - Vue opposée
    
    // Rotations discrètes (15° par pas)
    void rotateUp();          // Numpad 8
    void rotateDown();        // Numpad 2
    void rotateLeft();        // Numpad 4
    void rotateRight();       // Numpad 6

private:
    // ========================================================================
    // MÉTHODES PRIVÉES
    // ========================================================================
    
    void updateCameraVectors();
    void processMouseRotation(Window* window, float deltaTime);
    void processMousePan(Window* window, float deltaTime);
    void processMouseZoom(Window* window, float deltaTime);
    void processKeyboardShortcuts(Window* window);

    // ========================================================================
    // PARAMÈTRES DE POSITION
    // ========================================================================
    
    glm::vec3 m_target;       // Point pivot autour duquel on orbite
    float m_distance;         // Distance du pivot
    float m_yaw;              // Rotation horizontale (en degrés)
    float m_pitch;            // Rotation verticale (en degrés)
    
    glm::vec3 m_position;     // Position calculée de la caméra
    glm::vec3 m_front;        // Direction de vue
    glm::vec3 m_up;           // Vecteur haut
    glm::vec3 m_right;        // Vecteur droite
    glm::vec3 m_worldUp;      // Vecteur haut du monde

    // ========================================================================
    // PARAMÈTRES DE PROJECTION
    // ========================================================================
    
    ProjectionMode m_projectionMode;
    float m_fov;              // Field of view (perspective)
    float m_orthoSize;        // Taille de la vue orthographique
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // ========================================================================
    // PARAMÈTRES DE CONTRÔLE
    // ========================================================================
    
    float m_rotationSpeed;    // Sensibilité de rotation
    float m_panSpeed;         // Sensibilité de translation
    float m_zoomSpeed;        // Sensibilité du zoom
    float m_minDistance;      // Distance minimale du pivot
    float m_maxDistance;      // Distance maximale du pivot

    // État des inputs souris
    bool m_isRotating;        // MMB enfoncé
    bool m_isPanning;         // Shift + MMB
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;
    
    // Cooldown pour les touches numpad (éviter spam)
    float m_keyPressCooldown;
    float m_timeSinceLastKeyPress;
};