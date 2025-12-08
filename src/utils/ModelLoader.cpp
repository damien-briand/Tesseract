#include "ModelLoader.hpp"
#include "../rendering/Vertex.hpp"

// IMPORTANT : Mettre AVANT l'include du header tinyobj
#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Hash function pour Vertex (pour déduplication)
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<float>()(vertex.position.x);
            size_t h2 = hash<float>()(vertex.position. y);
            size_t h3 = hash<float>()(vertex.position.z);
            size_t h4 = hash<float>()(vertex.texCoord.x);
            size_t h5 = hash<float>()(vertex.texCoord.y);
            size_t h6 = hash<float>()(vertex.normal.x);
            size_t h7 = hash<float>()(vertex.normal.y);
            size_t h8 = hash<float>()(vertex.normal.z);
            
            return ((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ 
                   ((h4 << 3) ^ (h5 << 4)) ^
                   ((h6 << 5) ^ (h7 << 6) ^ (h8 << 7));
        }
    };
}

// Opérateur d'égalité pour Vertex
bool operator==(const Vertex& lhs, const Vertex& rhs) {
    return lhs.position == rhs.position && 
           lhs.color == rhs.color && 
           lhs.texCoord == rhs.texCoord &&
           lhs.normal == rhs.normal &&
           lhs.tangent == rhs.tangent;
}

/**
 * @brief Calcule les tangentes pour un ensemble de triangles
 * 
 * Cette fonction calcule les vecteurs tangents nécessaires pour le normal mapping. 
 * Elle utilise l'algorithme standard basé sur les coordonnées UV. 
 * 
 * @param vertices Vecteur de vertices (sera modifié avec les tangentes calculées)
 * @param indices Vecteur d'indices définissant les triangles
 */
void calculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    // Initialiser toutes les tangentes à zéro
    for (auto& vertex : vertices) {
        vertex.tangent = glm::vec3(0.0f);
    }

    // Calculer les tangentes pour chaque triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        // Récupérer les 3 vertices du triangle
        Vertex& v0 = vertices[indices[i + 0]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        // Edges du triangle dans l'espace monde
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        // Différences de coordonnées UV
        glm::vec2 deltaUV1 = v1. texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

        // Calcul de la tangente avec la formule standard
        // Éviter la division par zéro
        float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1. y;
        
        glm::vec3 tangent;
        if (std::abs(denom) > 0.00001f) {
            float f = 1.0f / denom;
            
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2. x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2. y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        } else {
            // Si les UVs sont dégénérées, utiliser un vecteur par défaut
            tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        // Accumuler les tangentes pour chaque vertex du triangle
        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    // Orthonormaliser les tangentes (processus de Gram-Schmidt)
    for (auto& vertex : vertices) {
        const glm::vec3& n = vertex.normal;
        glm::vec3& t = vertex.tangent;

        // Gram-Schmidt orthogonalisation
        // t = normalize(t - n * dot(n, t))
        t = glm::normalize(t - n * glm::dot(n, t));
        
        // Si la tangente est invalide (null ou NaN), utiliser une valeur par défaut
        if (glm::length(t) < 0.001f || std::isnan(t.x) || std::isnan(t.y) || std::isnan(t.z)) {
            // Créer une tangente perpendiculaire à la normale
            if (std::abs(n.x) > 0.9f) {
                t = glm::normalize(glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f)));
            } else {
                t = glm::normalize(glm::cross(n, glm::vec3(1.0f, 0.0f, 0.0f)));
            }
        }
    }
}

bool ModelLoader::loadModel(const std::string& filepath, 
                            std::vector<Vertex>& vertices, 
                            std::vector<uint32_t>& indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Charger le fichier . obj
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath. c_str())) {
        std::cerr << "✗ Failed to load model: " << filepath << std::endl;
        if (!err.empty()) {
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
                    attrib.vertices[3 * index. vertex_index + 0],
                    attrib.vertices[3 * index. vertex_index + 1],
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

            // Normale (si disponible dans le fichier OBJ)
            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                // Si pas de normale, on mettra une valeur par défaut
                // Elle sera recalculée plus tard
                vertex.normal = {0.0f, 0.0f, 1.0f};
            }

            // Couleur par défaut (blanc)
            vertex.color = {1.0f, 1.0f, 1.0f};

            // Tangente sera calculée plus tard
            vertex.tangent = {0.0f, 0.0f, 0.0f};

            // Déduplication
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices. push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    // Si le fichier OBJ n'avait pas de normales, on les calcule
    bool hasNormals = (attrib.normals.size() > 0);
    if (!hasNormals) {
        std::cout << "  ⚠ No normals in OBJ file, calculating flat normals..." << std::endl;
        
        // Calculer les normales de surface (flat shading)
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex& v0 = vertices[indices[i + 0]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0. position;
            glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

            // Assigner la même normale aux 3 vertices du triangle
            v0.normal = normal;
            v1.normal = normal;
            v2.normal = normal;
        }
    }

    // Calculer les tangentes pour TOUS les modèles (nécessaire pour normal mapping)
    std::cout << "Calculating tangents for normal mapping..." << std::endl;
    calculateTangents(vertices, indices);

    std::cout << "✓ Model loaded: " << filepath << std::endl;
    std::cout << "  Vertices: " << vertices.size() << std::endl;
    std::cout << "  Indices: " << indices.size() << std::endl;
    std::cout << "  Triangles: " << indices.size() / 3 << std::endl;
    std::cout << "  Normals: " << (hasNormals ? "from file" : "calculated") << std::endl;
    std::cout << "  Tangents: calculated" << std::endl;

    return true;
}