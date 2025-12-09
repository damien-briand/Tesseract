#pragma once

#include <glm/glm.hpp>

/**
 * @brief Structure contenant les données uniformes (MVP matrices)
 */
struct UniformBufferObject {
    alignas(16) glm::mat4 view;       // Transformation de la caméra
    alignas(16) glm::mat4 proj;       // Projection (perspective/ortho)
    alignas(16) glm::vec3 camPos;     // Position de la caméra
};