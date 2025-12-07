#pragma once
#include "Model.hpp"
#include <string>

class GltfLoader {
public:
    // Charge .gltf ou .glb et remplit Model
    static bool loadFromFile(const std::string& filename, Model& outModel);
};