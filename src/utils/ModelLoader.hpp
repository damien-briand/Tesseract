#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct Vertex;

/**
 * @brief Charge des modèles 3D depuis des fichiers . obj
 */
class ModelLoader {
public:
    /**
     * @brief Charge un modèle OBJ
     * @param filepath Chemin vers le fichier . obj
     * @param vertices Vertices chargés (OUTPUT)
     * @param indices Indices chargés (OUTPUT)
     * @return true si succès, false sinon
     */
    static bool loadModel(const std::string& filepath, 
                         std::vector<Vertex>& vertices, 
                         std::vector<uint32_t>& indices);
};