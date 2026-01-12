#include "PrimitiveGenerator.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

// ============================================================================
// GIZMO D'AXES 3D
// ============================================================================

void PrimitiveGenerator::generateAxisGizmo(float length, float thickness,
                                           std::vector<Vertex> &vertices,
                                           std::vector<uint32_t> &indices)
{
    vertices.clear();
    indices.clear();

    // Couleurs des axes
    glm::vec3 redColor(1.0f, 0.0f, 0.0f);   // X = Rouge
    glm::vec3 greenColor(0.0f, 1.0f, 0.0f); // Y = Vert
    glm::vec3 blueColor(0.0f, 0.0f, 1.0f);  // Z = Bleu

    // Segments pour les cylindres (8 segments = octogone)
    int segments = 8;

    // Axe X (rouge)
    generateCylinder(glm::vec3(0.0f, 0.0f, 0.0f),
                     glm::vec3(length, 0.0f, 0.0f),
                     thickness, redColor, segments,
                     vertices, indices);

    // Axe Y (vert)
    generateCylinder(glm::vec3(0.0f, 0.0f, 0.0f),
                     glm::vec3(0.0f, length, 0.0f),
                     thickness, greenColor, segments,
                     vertices, indices);

    // Axe Z (bleu)
    generateCylinder(glm::vec3(0.0f, 0.0f, 0.0f),
                     glm::vec3(0.0f, 0.0f, length),
                     thickness, blueColor, segments,
                     vertices, indices);
}

// ============================================================================
// CYLINDRE (utilisé pour les axes)
// ============================================================================

void PrimitiveGenerator::generateCylinder(const glm::vec3 &start, const glm::vec3 &end,
                                          float radius, const glm::vec3 &color,
                                          int segments,
                                          std::vector<Vertex> &vertices,
                                          std::vector<uint32_t> &indices)
{
    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

    // Calcul de l'axe du cylindre
    glm::vec3 axis = end - start;
    float height = glm::length(axis);
    glm::vec3 direction = glm::normalize(axis);

    // Trouver deux vecteurs perpendiculaires à l'axe
    glm::vec3 perpendicular1, perpendicular2;
    if (std::abs(direction.x) < 0.9f)
    {
        perpendicular1 = glm::normalize(glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    else
    {
        perpendicular1 = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    perpendicular2 = glm::cross(direction, perpendicular1);

    // Générer les vertices du cylindre
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / (float)segments * 2.0f * glm::pi<float>();
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);

        glm::vec3 offset = perpendicular1 * cosA * radius + perpendicular2 * sinA * radius;

        // Vertex au début
        Vertex v1;
        v1.position = start + offset;
        v1.color = color;
        v1.texCoord = glm::vec2((float)i / (float)segments, 0.0f);
        v1.normal = glm::normalize(offset);
        v1.tangent = direction;
        vertices.push_back(v1);

        // Vertex à la fin
        Vertex v2;
        v2.position = end + offset;
        v2.color = color;
        v2.texCoord = glm::vec2((float)i / (float)segments, 1.0f);
        v2.normal = glm::normalize(offset);
        v2.tangent = direction;
        vertices.push_back(v2);
    }

    // Générer les indices (triangles pour les faces latérales)
    for (int i = 0; i < segments; ++i)
    {
        uint32_t i0 = baseIndex + i * 2;
        uint32_t i1 = i0 + 1;
        uint32_t i2 = i0 + 2;
        uint32_t i3 = i0 + 3;

        // Triangle 1
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);

        // Triangle 2
        indices.push_back(i2);
        indices.push_back(i1);
        indices.push_back(i3);
    }

    // Ajouter des caps (cercles aux extrémités) - optionnel pour un gizmo
    // Pour simplifier, on peut les omettre car les axes sont fins
}

// ============================================================================
// CUBE UNITAIRE
// ============================================================================

void PrimitiveGenerator::generateCube(std::vector<Vertex> &vertices,
                                      std::vector<uint32_t> &indices)
{
    vertices.clear();
    indices.clear();

    // Couleur par défaut (blanc)
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    // Définition des 8 sommets d'un cube unitaire centré
    glm::vec3 positions[8] = {
        {-0.5f, -0.5f, -0.5f}, // 0
        {0.5f, -0.5f, -0.5f},  // 1
        {0.5f, 0.5f, -0.5f},   // 2
        {-0.5f, 0.5f, -0.5f},  // 3
        {-0.5f, -0.5f, 0.5f},  // 4
        {0.5f, -0.5f, 0.5f},   // 5
        {0.5f, 0.5f, 0.5f},    // 6
        {-0.5f, 0.5f, 0.5f}    // 7
    };

    // Normales des 6 faces
    glm::vec3 normals[6] = {
        {0.0f, 0.0f, -1.0f}, // Face avant
        {0.0f, 0.0f, 1.0f},  // Face arrière
        {-1.0f, 0.0f, 0.0f}, // Face gauche
        {1.0f, 0.0f, 0.0f},  // Face droite
        {0.0f, -1.0f, 0.0f}, // Face bas
        {0.0f, 1.0f, 0.0f}   // Face haut
    };

    // Coordonnées de texture standard
    glm::vec2 texCoords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}};

    // Indices des vertices pour chaque face (dans l'ordre anti-horaire)
    int faceIndices[6][4] = {
        {0, 1, 2, 3}, // Face avant (-Z)
        {5, 4, 7, 6}, // Face arrière (+Z)
        {4, 0, 3, 7}, // Face gauche (-X)
        {1, 5, 6, 2}, // Face droite (+X)
        {4, 5, 1, 0}, // Face bas (-Y)
        {3, 2, 6, 7}  // Face haut (+Y)
    };

    // Générer les vertices et indices pour chaque face
    for (int face = 0; face < 6; ++face)
    {
        uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

        for (int i = 0; i < 4; ++i)
        {
            Vertex v;
            v.position = positions[faceIndices[face][i]];
            v.color = color;
            v.texCoord = texCoords[i];
            v.normal = normals[face];
            v.tangent = glm::vec3(1.0f, 0.0f, 0.0f); // Tangente par défaut
            vertices.push_back(v);
        }

        // Deux triangles par face
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);

        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    }
}

// ============================================================================
// SPHÈRE UV
// ============================================================================

void PrimitiveGenerator::generateSphere(float radius, int segments,
                                        std::vector<Vertex> &vertices,
                                        std::vector<uint32_t> &indices)
{
    vertices.clear();
    indices.clear();

    glm::vec3 color(1.0f, 1.0f, 1.0f);

    // Générer les vertices
    for (int lat = 0; lat <= segments; ++lat)
    {
        float theta = (float)lat / (float)segments * glm::pi<float>();
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= segments; ++lon)
        {
            float phi = (float)lon / (float)segments * 2.0f * glm::pi<float>();
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            // Position
            glm::vec3 position(
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi);

            // Normale (normalisée)
            glm::vec3 normal = glm::normalize(position);

            // Coordonnées de texture
            glm::vec2 texCoord(
                (float)lon / (float)segments,
                (float)lat / (float)segments);

            // Tangente (direction phi)
            glm::vec3 tangent(
                -sinPhi,
                0.0f,
                cosPhi);

            Vertex v;
            v.position = position;
            v.color = color;
            v.texCoord = texCoord;
            v.normal = normal;
            v.tangent = glm::normalize(tangent);
            vertices.push_back(v);
        }
    }

    // Générer les indices
    for (int lat = 0; lat < segments; ++lat)
    {
        for (int lon = 0; lon < segments; ++lon)
        {
            uint32_t first = lat * (segments + 1) + lon;
            uint32_t second = first + segments + 1;

            // Triangle 1
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            // Triangle 2
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void PrimitiveGenerator::generateXZQuad(float size, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    vertices.clear();
    indices.clear();
    float h = size * 0.5f;
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    // 4 sommets (XZ)
    vertices.push_back({glm::vec3(-h, 0, -h), color, glm::vec2(0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)});
    vertices.push_back({glm::vec3(h, 0, -h), color, glm::vec2(1, 0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)});
    vertices.push_back({glm::vec3(h, 0, h), color, glm::vec2(1, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)});
    vertices.push_back({glm::vec3(-h, 0, h), color, glm::vec2(0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0)});
    // 2 triangles
    indices = {0, 1, 2, 2, 3, 0};
}