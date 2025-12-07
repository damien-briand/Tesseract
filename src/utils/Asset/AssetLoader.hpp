#pragma once
#include "Model.hpp"
#include <string>

class AssetLoader {
public:
    // Detecte par extension et appelle le loader adapté
    // Retourne vrai si succès et remplit model
    static bool loadAsset(const std::string& path, Model& model);

    // Expose les loaders si besoin
    static bool loadObj(const std::string& path, Model& model);
    static bool loadGltf(const std::string& path, Model& model);
};