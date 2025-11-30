#pragma once

#include <vulkan/vulkan.h>
#include <array>

// GLM pour les vecteurs (déjà dans votre CMakeLists)
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 pos;      // Position 2D
    glm::vec3 color;    // Couleur RGB

    /**
     * @brief Retourne la description du binding (un seul binding = 0)
     * Décrit comment les données sont organisées en mémoire
     */
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;                          // Index du binding
        bindingDescription.stride = sizeof(Vertex);              // Taille d'un vertex
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // Par vertex (pas par instance)

        return bindingDescription;
    }

    /**
     * @brief Retourne les descriptions des attributs (position + couleur)
     * Décrit comment extraire les données de position et couleur
     */
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        // Attribut 0 : Position
        attributeDescriptions[0].binding = 0;                    // Binding d'où viennent les données
        attributeDescriptions[0].location = 0;                   // location dans le shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;  // vec2 (2 floats)
        attributeDescriptions[0].offset = offsetof(Vertex, pos);    // Offset dans la structure

        // Attribut 1 : Couleur
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 (3 floats)
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};