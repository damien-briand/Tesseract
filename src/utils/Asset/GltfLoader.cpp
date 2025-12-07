#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "GltfLoader.hpp"
#include "TextureLoader.hpp"
#include <iostream>
#include <unordered_map>
#include <filesystem>

bool GltfLoader::loadFromFile(const std::string& filename, Model& outModel) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret;
    std::string ext = filename.substr(filename.find_last_of('.') + 1);
    if (ext == "glb") {
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
    } else {
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
    }

    if (!warn.empty()) {
        std::cout << "gltf warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "gltf error: " << err << std::endl;
    }
    if (!ret) return false;

    namespace fs = std::filesystem;
    fs::path basePath = fs::path(filename).parent_path();

    // Map tinygltf image index -> outModel.texture index
    std::vector<int> imageToTextureIndex;
    imageToTextureIndex.resize(gltfModel.images.size(), -1);

    // Load images: prefer external files using TextureLoader (to support KTX2)
    for (size_t i = 0; i < gltfModel.images.size(); ++i) {
        const tinygltf::Image &img = gltfModel.images[i];
        int texIndex = -1;

        if (!img.uri.empty()) {
            // data uri or external file
            if (img.uri.rfind("data:", 0) == 0) {
                // embedded data URI -> tinygltf has decoded it into img.image
                if (!img.image.empty()) {
                    TextureImage t;
                    t.path = filename + "#image" + std::to_string(i);
                    t.width = img.width;
                    t.height = img.height;
                    t.mipLevels = 1;
                    t.isKtx = false;
                    t.isCompressed = false;
                    t.pixels = img.image; // tinygltf stores decoded bytes here
                    outModel.textures.push_back(std::move(t));
                    texIndex = static_cast<int>(outModel.textures.size()) - 1;
                }
            } else {
                // external file -> resolve path and use TextureLoader (handles KTX2)
                fs::path imgPath = basePath / img.uri;
                std::string imgPathStr = imgPath.string();
                if (!TextureLoader::loadTextureToModel(imgPathStr, outModel, texIndex)) {
                    std::cerr << "GltfLoader: failed to load external image: " << imgPathStr << std::endl;
                } else {
                    // mark sRGB for baseColor textures later when mapping materials
                }
            }
        } else if (!img.image.empty()) {
            // image loaded into memory by tinygltf (binary glb embedded images)
            TextureImage t;
            t.path = filename + "#image" + std::to_string(i);
            t.width = img.width;
            t.height = img.height;
            t.mipLevels = 1;
            t.isKtx = false;
            t.isCompressed = false;
            t.pixels = img.image; // decoded pixels
            outModel.textures.push_back(std::move(t));
            texIndex = static_cast<int>(outModel.textures.size()) - 1;
        }

        imageToTextureIndex[i] = texIndex;
    }

    // Materials
    outModel.materials.reserve(gltfModel.materials.size());
    for (const auto& m : gltfModel.materials) {
        Material mat;
        mat.name = m.name;
        // pbrMetallicRoughness
        if (m.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            int imgIndex = m.pbrMetallicRoughness.baseColorTexture.index;
            if (imgIndex < (int)imageToTextureIndex.size()) {
                mat.baseColorTextureIndex = imageToTextureIndex[imgIndex];
                // baseColor textures are color textures -> mark isSRGB if possible
                if (mat.baseColorTextureIndex >= 0 && mat.baseColorTextureIndex < (int)outModel.textures.size()) {
                    outModel.textures[mat.baseColorTextureIndex].isSRGB = true;
                }
            }
        }
        auto factor = m.pbrMetallicRoughness.baseColorFactor;
        mat.baseColorFactor = glm::vec4(
            factor.size() > 0 ? factor[0] : 1.0f,
            factor.size() > 1 ? factor[1] : 1.0f,
            factor.size() > 2 ? factor[2] : 1.0f,
            factor.size() > 3 ? factor[3] : 1.0f
        );
        outModel.materials.push_back(std::move(mat));
    }

    // Meshes (iterate gltfModel.meshes -> primitives) — unchanged reading of positions/normals/texcoords/indices
    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& prim : mesh.primitives) {
            Mesh outMesh;
            // ... (same code as before, building vertices and indices) ...
            // For brevity we reuse the previously implemented accessor readers
            auto readAccessorFloatVec3 = [&](int accessorIdx, std::vector<glm::vec3>& dest){
                if (accessorIdx < 0) return;
                const tinygltf::Accessor& acc = gltfModel.accessors[accessorIdx];
                const tinygltf::BufferView& bv = gltfModel.bufferViews[acc.bufferView];
                const tinygltf::Buffer& buf = gltfModel.buffers[bv.buffer];
                const unsigned char* dataPtr = buf.data.data() + bv.byteOffset + acc.byteOffset;
                dest.resize(acc.count);
                for (size_t i = 0; i < acc.count; ++i) {
                    const float* f = reinterpret_cast<const float*>(dataPtr + i * acc.ByteStride(bv));
                    dest[i] = glm::vec3(f[0], f[1], f[2]);
                }
            };
            auto readAccessorFloatVec2 = [&](int accessorIdx, std::vector<glm::vec2>& dest){
                if (accessorIdx < 0) return;
                const tinygltf::Accessor& acc = gltfModel.accessors[accessorIdx];
                const tinygltf::BufferView& bv = gltfModel.bufferViews[acc.bufferView];
                const tinygltf::Buffer& buf = gltfModel.buffers[bv.buffer];
                const unsigned char* dataPtr = buf.data.data() + bv.byteOffset + acc.byteOffset;
                dest.resize(acc.count);
                for (size_t i = 0; i < acc.count; ++i) {
                    const float* f = reinterpret_cast<const float*>(dataPtr + i * acc.ByteStride(bv));
                    dest[i] = glm::vec2(f[0], f[1]);
                }
            };

            std::vector<glm::vec3> positions, normals;
            std::vector<glm::vec2> texcoords;
            if (prim.attributes.find("POSITION") != prim.attributes.end()) {
                readAccessorFloatVec3(prim.attributes.at("POSITION"), positions);
            }
            if (prim.attributes.find("NORMAL") != prim.attributes.end()) {
                readAccessorFloatVec3(prim.attributes.at("NORMAL"), normals);
            }
            if (prim.attributes.find("TEXCOORD_0") != prim.attributes.end()) {
                readAccessorFloatVec2(prim.attributes.at("TEXCOORD_0"), texcoords);
            }

            // indices
            std::vector<uint32_t> indices;
            if (prim.indices >= 0) {
                const tinygltf::Accessor& acc = gltfModel.accessors[prim.indices];
                const tinygltf::BufferView& bv = gltfModel.bufferViews[acc.bufferView];
                const tinygltf::Buffer& buf = gltfModel.buffers[bv.buffer];
                const unsigned char* dataPtr = buf.data.data() + bv.byteOffset + acc.byteOffset;
                indices.resize(acc.count);
                for (size_t i = 0; i < acc.count; ++i) {
                    if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const uint16_t* p = reinterpret_cast<const uint16_t*>(dataPtr + i * acc.ByteStride(bv));
                        indices[i] = static_cast<uint32_t>(*p);
                    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const uint32_t* p = reinterpret_cast<const uint32_t*>(dataPtr + i * acc.ByteStride(bv));
                        indices[i] = *p;
                    } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        const uint8_t* p = reinterpret_cast<const uint8_t*>(dataPtr + i * acc.ByteStride(bv));
                        indices[i] = static_cast<uint32_t>(*p);
                    } else {
                        std::cerr << "Unsupported index component type\n";
                    }
                }
            } else {
                indices.resize(positions.size());
                for (size_t i = 0; i < positions.size(); ++i) indices[i] = static_cast<uint32_t>(i);
            }

            // build vertices
            outMesh.vertices.resize(positions.size());
            for (size_t i = 0; i < positions.size(); ++i) {
                Vertex v;
                v.position = positions[i];
                v.normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f, 0.0f, 1.0f);
                v.texCoord = (i < texcoords.size()) ? texcoords[i] : glm::vec2(0.0f, 0.0f);
                v.color = glm::vec3(1.0f);
                outMesh.vertices[i] = v;
            }
            outMesh.indices = std::move(indices);
            outMesh.materialIndex = prim.material;
            outModel.meshes.push_back(std::move(outMesh));
        }
    }

    return true;
}