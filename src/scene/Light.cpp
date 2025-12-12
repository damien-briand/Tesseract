#include "Light.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Light::Light(const std::string &name, Type type)
    : m_name(name), m_type(type), m_position(0.0f), m_direction(0.0f, -1.0f, 0.0f) // Vers le bas par défaut
      ,
      m_color(1.0f), m_intensity(1.0f), m_range(10.0f), m_innerConeAngle(glm::radians(15.0f)), m_outerConeAngle(glm::radians(30.0f)), m_enabled(true)
{
}

Light::GPUData Light::toGPUData() const
{
    GPUData data{};

    // Position ou direction selon le type
    if (m_type == Type::Directional)
    {
        data.position = glm::vec4(m_direction, 0.0f); // w=0 indique une direction
    }
    else
    {
        data.position = glm::vec4(m_position, 1.0f); // w=1 indique une position
    }

    // Couleur avec intensité
    data.color = glm::vec4(m_color * m_intensity, 1.0f);

    // Light space matrix (identité pour l'instant, utilisé pour shadow mapping plus tard)
    data.lightSpaceMatrix = glm::mat4(1.0f);

    // Type
    data.lightType = static_cast<int32_t>(m_type);

    // Paramètres
    data.range = m_range;
    data.innerConeAngle = m_innerConeAngle;
    data.outerConeAngle = m_outerConeAngle;

    return data;
}

Light *Light::createDirectional(const std::string &name, const glm::vec3 &direction, const glm::vec3 &color, float intensity)
{
    Light *light = new Light(name, Type::Directional);
    light->setDirection(direction);
    light->setColor(color);
    light->setIntensity(intensity);
    std::cout << "✓ Directional light '" << name << "' created" << std::endl;
    return light;
}

Light *Light::createPoint(const std::string &name, const glm::vec3 &position, const glm::vec3 &color, float intensity, float range)
{
    Light *light = new Light(name, Type::Point);
    light->setPosition(position);
    light->setColor(color);
    light->setIntensity(intensity);
    light->setRange(range);
    std::cout << "✓ Point light '" << name << "' created" << std::endl;
    return light;
}

Light *Light::createSpot(const std::string &name, const glm::vec3 &position, const glm::vec3 &direction,
                         const glm::vec3 &color, float intensity, float range, float innerAngle, float outerAngle)
{
    Light *light = new Light(name, Type::Spot);
    light->setPosition(position);
    light->setDirection(direction);
    light->setColor(color);
    light->setIntensity(intensity);
    light->setRange(range);
    light->setInnerConeAngle(innerAngle);
    light->setOuterConeAngle(outerAngle);
    std::cout << "✓ Spot light '" << name << "' created" << std::endl;
    return light;
}