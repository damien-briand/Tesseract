#include "TextureLoader.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <string>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

// Optional: include libktx loader if you add it to CMake
// #include <ktx.h>

bool TextureLoader::loadTextureToModel(const std::string &texturePath, Model &model, int &outTextureIndex)
{
    namespace fs = std::filesystem;
    std::string ext = fs::path(texturePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // KTX2 branch (example placeholder) : if libktx present you would call ktxTexture_CreateFromNamedFile and extract data
    if (ext == ".ktx2")
    {
        std::cerr << "KTX2 support requested: compile with libktx and implement loader (placeholder)\n";
        return false; // or implement using libktx
    }

    // Fallback to stb_image
    int w, h, ch;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (!pixels)
    {
        std::cerr << "Failed to load texture via stb: " << texturePath << std::endl;
        return false;
    }
    TextureImage img;
    img.width = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.mipLevels = 1;
    img.path = texturePath;
    size_t size = (size_t)w * h * 4;
    img.pixels.resize(size);
    memcpy(img.pixels.data(), pixels, size);
    stbi_image_free(pixels);

    model.textures.push_back(std::move(img));
    outTextureIndex = static_cast<int>(model.textures.size()) - 1;
    return true;
}