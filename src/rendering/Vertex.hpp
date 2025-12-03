#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

/**
 * @brief Structure représentant un vertex avec position, couleur et coordonnées de texture
 */
struct Vertex {
    glm::vec2 position;  // Position 2D (x, y)
    glm::vec3 color;     // Couleur RGB
    glm::vec2 texCoord;  // Coordonnées de texture (u, v)

    /**
     * @brief Retourne la description du binding (comment les données sont organisées)
     * @return VkVertexInputBindingDescription
     */
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;                          // Index du binding
        bindingDescription.stride = sizeof(Vertex);              // Taille d'un vertex
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Par vertex (pas par instance)

        return bindingDescription;
    }

    /**
     * @brief Retourne les descriptions des attributs (position, couleur, texCoord)
     * @return Array de VkVertexInputAttributeDescription
     */
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        // Attribut 0 : Position
        attributeDescriptions[0].binding = 0;                    // Binding index
        attributeDescriptions[0].location = 0;                   // Location dans le shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 (3 floats)
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        // Attribut 1 : Couleur
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 (3 floats)
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        // Attribut 2 : Coordonnées de texture
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // vec2 (2 floats)
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};