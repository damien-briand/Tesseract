#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

// Vertex identique à votre Vertex actuel ; on l'utilise pour la compatibilité
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

struct TextureImage {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 1;
    std::vector<unsigned char> pixels; // raw RGBA8 if loaded via stb
    std::string path;                  // original path
    // For KTX2 or GPU-only textures you may want to store VkImage/VkImageView later
};

struct Material {
    std::string name;
    int baseColorTextureIndex = -1; // index into textures
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    // add other properties as needed (metallicRoughness, normal, etc.)
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    int materialIndex = -1;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<TextureImage> textures; // CPU-side images; TextureLoader may also create GPU resources
    std::vector<Material> materials;
};