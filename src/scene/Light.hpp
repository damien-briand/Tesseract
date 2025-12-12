#pragma once

#include <glm/glm.hpp>
#include <string>

/**
 * @brief Représente une source lumineuse dans la scène
 * 
 * Types de lumières supportés :
 * - Directional : Lumière directionnelle (soleil), éclaire uniformément depuis une direction
 * - Point :  Lumière omnidirectionnelle avec atténuation (lampe, bougie)
 * - Spot : Lumière conique (projecteur, lampe de poche)
 * - Ambient : Lumière ambiante globale (pas une vraie lumière, juste un terme constant)
 */
class Light {
public:
    enum class Type {
        Directional = 1,  // Soleil, lune
        Point = 0,        // Lampe, torche
        Spot = 2,         // Projecteur
        Ambient = -1      // Éclairage ambiant (pas dans le buffer)
    };

    /**
     * @brief Constructeur
     * @param name Nom de la lumière
     * @param type Type de lumière
     */
    Light(const std::string& name, Type type);

    // Getters/Setters pour la position/direction
    void setPosition(const glm::vec3& position) { m_position = position; }
    const glm::vec3& getPosition() const { return m_position; }

    void setDirection(const glm::vec3& direction) { m_direction = glm::normalize(direction); }
    const glm::vec3& getDirection() const { return m_direction; }

    // Couleur et intensité
    void setColor(const glm::vec3& color) { m_color = color; }
    const glm::vec3& getColor() const { return m_color; }

    void setIntensity(float intensity) { m_intensity = intensity; }
    float getIntensity() const { return m_intensity; }

    // Paramètres spécifiques
    void setRange(float range) { m_range = range; }  // Pour Point et Spot
    float getRange() const { return m_range; }

    void setInnerConeAngle(float angle) { m_innerConeAngle = angle; }  // Pour Spot
    float getInnerConeAngle() const { return m_innerConeAngle; }

    void setOuterConeAngle(float angle) { m_outerConeAngle = angle; }  // Pour Spot
    float getOuterConeAngle() const { return m_outerConeAngle; }

    // Contrôle
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Type et nom
    Type getType() const { return m_type; }
    const std:: string& getName() const { return m_name; }

    /**
     * @brief Structure de données pour le storage buffer GPU
     * Doit correspondre exactement à la structure dans le shader
     */
    struct GPUData {
        alignas(16) glm::vec4 position;          // xyz = position, w = 1.0 (ou direction si directional)
        alignas(16) glm::vec4 color;             // rgb = color * intensity, a = unused
        alignas(16) glm::mat4 lightSpaceMatrix;  // Pour shadow mapping (future feature)
        alignas(4) int32_t lightType;            // 0=Point, 1=Directional, 2=Spot
        alignas(4) float range;
        alignas(4) float innerConeAngle;
        alignas(4) float outerConeAngle;
    };

    /**
     * @brief Génère les données GPU pour cette lumière
     */
    GPUData toGPUData() const;

    /**
     * @brief Crée une lumière directionnelle (soleil)
     */
    static Light* createDirectional(const std::string& name, const glm::vec3& direction, const glm::vec3& color, float intensity);

    /**
     * @brief Crée une lumière ponctuelle (lampe)
     */
    static Light* createPoint(const std::string& name, const glm::vec3& position, const glm::vec3& color, float intensity, float range);

    /**
     * @brief Crée une lumière spot (projecteur)
     */
    static Light* createSpot(const std::string& name, const glm::vec3& position, const glm::vec3& direction, 
                            const glm::vec3& color, float intensity, float range, float innerAngle, float outerAngle);

private:
    std::string m_name;
    Type m_type;

    // Transform
    glm::vec3 m_position;
    glm:: vec3 m_direction;

    // Appearance
    glm::vec3 m_color;
    float m_intensity;

    // Atténuation (Point et Spot)
    float m_range;

    // Spot parameters
    float m_innerConeAngle;  // En radians
    float m_outerConeAngle;  // En radians

    // State
    bool m_enabled;
};