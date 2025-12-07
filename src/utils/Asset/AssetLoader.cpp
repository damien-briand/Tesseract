#include "AssetLoader.hpp"
#include "GltfLoader.hpp"
#include "../ModelLoader.hpp"
#include <algorithm>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool AssetLoader::loadAsset(const std::string& path, Model& model) {
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".obj") {
        return loadObj(path, model);
    } else if (ext == ".gltf" || ext == ".glb") {
        return loadGltf(path, model);
    } else {
        std::cerr << "Unsupported asset extension: " << ext << std::endl;
        return false;
    }
}

bool AssetLoader::loadObj(const std::string& path, Model& model) {
    // Reuse your existing ModelLoader but you'll need a small adapter to fill Model struct.
    // Suppose existing ModelLoader::loadModel(filepath, std::vector<Vertex>&, std::vector<uint32_t>&)
    std::vector<Vertex> vtx;
    std::vector<uint32_t> idx;
    if (!ModelLoader::loadModel(path, vtx, idx)) {
        return false;
    }
    // Simple single-mesh packing (OBJ can have multiple shapes; you can extend)
    Mesh mesh;
    mesh.vertices = std::move(vtx);
    mesh.indices = std::move(idx);
    model.meshes.push_back(std::move(mesh));
    // Textures/materials: your current OBJ loader probably doesn't fill textures — you can extend it later
    return true;
}

bool AssetLoader::loadGltf(const std::string& path, Model& model) {
    return GltfLoader::loadFromFile(path, model);
}