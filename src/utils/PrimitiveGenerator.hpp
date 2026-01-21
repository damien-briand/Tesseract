#pragma once

#include <vector>
#include "../rendering/Vertex.hpp"

/**
 * @brief Générateur de géométries primitives (axes, cubes, sphères, etc.)
 */
class PrimitiveGenerator
{
public:
    /**
     * @brief Génère un gizmo d'axes 3D (X=rouge, Y=vert, Z=bleu)
     * @param length Longueur de chaque axe
     * @param thickness Épaisseur des lignes (simulations par cylindres fins)
     * @param vertices [out] Vertices du mesh
     * @param indices [out] Indices du mesh
     */
    static void generateAxisGizmo(float length, float thickness,
                                  std::vector<Vertex> &vertices,
                                  std::vector<uint32_t> &indices);

    /**
     * @brief Génère un cube unitaire (1x1x1 centré à l'origine)
     * @param vertices [out] Vertices du mesh
     * @param indices [out] Indices du mesh
     */
    static void generateCube(std::vector<Vertex> &vertices,
                             std::vector<uint32_t> &indices);

    /**
     * @brief Génère une sphère UV
     * @param radius Rayon de la sphère
     * @param segments Nombre de segments (résolution)
     * @param vertices [out] Vertices du mesh
     * @param indices [out] Indices du mesh
     */
    static void generateSphere(float radius, int segments,
                               std::vector<Vertex> &vertices,
                               std::vector<uint32_t> &indices);

    /**
     * @brief Génère un quad sur le plan XZ centré à l'origine
     * @param size Taille du quad (longueur d'un côté)
     * @param vertices [out] Vertices du mesh
     * @param indices [out] Indices du mesh
     */
    static void generateXZQuad(float size, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

private:
    /**
     * @brief Génère un cylindre (utilisé pour représenter les axes)
     * @param start Point de départ
     * @param end Point d'arrivée
     * @param radius Rayon du cylindre
     * @param color Couleur
     * @param segments Nombre de segments circulaires
     * @param vertices [out] Vertices du mesh
     * @param indices [out] Indices du mesh
     */
    static void generateCylinder(const glm::vec3 &start, const glm::vec3 &end,
                                 float radius, const glm::vec3 &color,
                                 int segments,
                                 std::vector<Vertex> &vertices,
                                 std::vector<uint32_t> &indices);
};
