#pragma once

#include <glm/glm.hpp>

/**
 * @brief Structure contenant les données uniformes globales
 * 
 * Maintenant avec support des lumières : 
 * - Matrices de transformation (view, proj)
 * - Position de la caméra
 * - Nombre de lumières actives
 * - Couleur ambiante
 * - Paramètres de rendu (exposure, IBL scale, etc.)
 */
struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 camPos;
    alignas(4) int lightCount;              // Nombre de lumières dans le storage buffer
    alignas(16) glm::vec3 ambientColor;     // Couleur ambiante globale
    alignas(4) float ambientIntensity;      // Intensité ambiante
    alignas(4) float exposure;              // Pour HDR tone mapping (défaut:  1.0)
    alignas(4) float scaleIBLAmbient;       // Scale pour IBL ambient (défaut:  1.0)
};