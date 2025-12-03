#include "ModelLoader.hpp"
#include "../rendering/Vertex.hpp"

// IMPORTANT : Mettre AVANT l'include du header tinyobj
#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>

// Hash function pour Vertex (pour déduplication)
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<float>()(vertex.position.x);
            size_t h2 = hash<float>()(vertex.position.y);
            size_t h3 = hash<float>()(vertex.position.z);
            size_t h4 = hash<float>()(vertex.texCoord.x);
            size_t h5 = hash<float>()(vertex.texCoord.y);
            
            return ((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ ((h4 << 3) ^ (h5 << 4));
        }
    };
}

// Opérateur d'égalité pour Vertex
bool operator==(const Vertex& lhs, const Vertex& rhs) {
    return lhs. position == rhs.position && 
           lhs.color == rhs.color && 
           lhs.texCoord == rhs.texCoord;
}

bool ModelLoader::loadModel(const std::string& filepath, 
                            std::vector<Vertex>& vertices, 
                            std::vector<uint32_t>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Charger le fichier . obj
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        std::cerr << "✗ Failed to load model: " << filepath << std::endl;
        if (! err.empty()) {
            std::cerr << "  Error: " << err << std::endl;
        }
        return false;
    }

    if (! warn.empty()) {
        std::cout << "⚠ Warning: " << warn << std::endl;
    }

    // Map pour déduplication
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    // Parcourir toutes les shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            // Position
            if (index.vertex_index >= 0) {
                vertex. position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
            }

            // Texture coordinates (si disponibles)
            if (index.texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index. texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index. texcoord_index + 1]  // Inverser V pour Vulkan
                };
            } else {
                vertex.texCoord = {0.0f, 0.0f};
            }

            // Couleur par défaut (blanc)
            vertex.color = {1.0f, 1.0f, 1.0f};

            // Déduplication
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices. push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    std::cout << "✓ Model loaded: " << filepath << std::endl;
    std::cout << "  Vertices: " << vertices.size() << std::endl;
    std::cout << "  Indices: " << indices.size() << std::endl;
    std::cout << "  Triangles: " << indices.size() / 3 << std::endl;

    return true;
}