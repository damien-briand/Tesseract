#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

/**
 * @brief Structure représentant un vertex avec position 3D, couleur, coordonnées de texture, normale et tangente
 */
struct Vertex {
    glm::vec3 position;  // Position 3D (x, y, z)
    glm::vec3 color;     // Couleur RGB
    glm::vec2 texCoord;  // Coordonnées de texture (u, v)
    glm::vec3 normal;    // Normale (pour l'éclairage)
    glm::vec3 tangent;   // Tangente (pour les normal maps)

    /**
     * @brief Retourne la description du binding (comment les données sont organisées)
     * @return VkVertexInputBindingDescription
     */
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);  // Maintenant : 3+3+2+3+3 = 14 floats = 56 octets
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    /**
     * @brief Retourne les descriptions des attributs (position, couleur, texCoord, normal, tangent)
     * @return Array de VkVertexInputAttributeDescription
     */
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

        // Attribut 0 : Position (vec3)
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        // Attribut 1 : Couleur (vec3)
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1]. format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // Attribut 2 : Coordonnées de texture (vec2)
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        // Attribut 3 : Normale (vec3)
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        // Attribut 4 : Tangente (vec3)
        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4]. location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, tangent);

        return attributeDescriptions;
    }
};