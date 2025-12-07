#include "TextureLoader.hpp"

#include <iostream>
#include <filesystem>
#include <algorithm> // std::transform
#include <cctype>    // std::tolower
#include <string>
#include <cstring>   // memcpy

#include "../external/stb_image.h"

#ifdef WITH_KTX
  #include <ktx.h>
#endif

bool TextureLoader::loadTextureToModel(const std::string& texturePath, Model& model, int& outTextureIndex) {
    namespace fs = std::filesystem;
    std::string ext = fs::path(texturePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (ext == ".ktx2") {
#ifdef WITH_KTX
        // Try to load KTX2 via libktx
        ktxTexture* kTexture = nullptr;
        KTX_error_code result = ktxTexture_CreateFromNamedFile(texturePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
        if (result != KTX_SUCCESS || kTexture == nullptr) {
            std::cerr << "libktx: failed to load KTX texture: " << texturePath << " error: " << result << std::endl;
            if (kTexture) ktxTexture_Destroy(kTexture);
            return false;
        }

        // Get info
        ktx_uint32_t levels = ktxTexture_GetNumLevels(kTexture);
        ktx_uint32_t width = ktxTexture_GetWidth(kTexture, 0);
        ktx_uint32_t height = ktxTexture_GetHeight(kTexture, 0);
        ktx_size_t dataSize = ktxTexture_GetDataSize(kTexture);

        TextureImage img;
        img.path = texturePath;
        img.mipLevels = levels;
        img.width = static_cast<uint32_t>(width);
        img.height = static_cast<uint32_t>(height);
        img.isKtx = true;

        const ktx_uint8_t* data = ktxTexture_GetData(kTexture);
        if (data && dataSize > 0) {
            // Note: data may be decoded pixels (RGBA8) OR compressed payload (Basis/ASTC/..)
            // We copy the blob. Later, Texture::createTextureImage will decide how to upload it.
            img.pixels.resize(dataSize);
            std::memcpy(img.pixels.data(), data, dataSize);

            // Heuristic: if size == width*height*4*levels -> probably decoded RGBA8
            if (dataSize == static_cast<ktx_size_t>(width) * height * 4 * levels) {
                img.isCompressed = false;
            } else {
                img.isCompressed = true; // compressed blob (most KTX2 with BasisU)
            }
        } else {
            std::cerr << "libktx: no accessible image data in " << texturePath << std::endl;
            ktxTexture_Destroy(kTexture);
            return false;
        }

        model.textures.push_back(std::move(img));
        outTextureIndex = static_cast<int>(model.textures.size()) - 1;

        ktxTexture_Destroy(kTexture);
        return true;
#else
        std::cerr << "KTX2 support disabled (libktx not found at build time). Rebuild with libktx and WITH_KTX=1.\n";
        return false;
#endif
    }

    // fallback to stb_image for PNG/JPEG/etc.
    int w = 0, h = 0, ch = 0;
    stbi_uc* pixels = stbi_load(texturePath.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (!pixels) {
        std::cerr << "stb_image: failed to load: " << texturePath << std::endl;
        return false;
    }

    TextureImage img;
    img.width = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.mipLevels = 1;
    img.path = texturePath;
    img.isKtx = false;
    img.isCompressed = false;
    size_t size = static_cast<size_t>(w) * static_cast<size_t>(h) * 4;
    img.pixels.resize(size);
    std::memcpy(img.pixels.data(), pixels, size);
    stbi_image_free(pixels);

    model.textures.push_back(std::move(img));
    outTextureIndex = static_cast<int>(model.textures.size()) - 1;
    return true;
}