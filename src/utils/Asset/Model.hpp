#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

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
    std::vector<unsigned char> pixels; // raw bytes (decoded RGBA8 or raw KTX blob)
    std::string path;                  // original path or uri
    bool isKtx = false;                // provient d'un fichier KTX2
    bool isCompressed = false;         // true si les bytes sont un format compressé (e.g. BasisU)
    bool isSRGB = false;               // true si texture color (baseColor) -> use SRGB format on GPU
};
struct Material {
    std::string name;
    int baseColorTextureIndex = -1; // index into textures
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    // add other properties as needed
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