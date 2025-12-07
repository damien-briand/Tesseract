#include "TextureLoader.hpp"
#include "../external/stb_image.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifdef WITH_KTX
#include <ktx.h>
#include <ktxvulkan.h>
#endif

bool TextureLoader::loadTextureToModel(const std::string &texturePath, Model &model, int &outTextureIndex)
{
    namespace fs = std::filesystem;
    std::string ext = fs::path(texturePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });

    if (ext == ".ktx2")
    {
#ifdef WITH_KTX
        // Try to load KTX2 with libktx
        ktxTexture *kTexture = nullptr;
        KTX_error_code result = ktxTexture_CreateFromNamedFile(texturePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
        if (result != KTX_SUCCESS || kTexture == nullptr)
        {
            std::cerr << "libktx: failed to load KTX texture: " << texturePath << " error: " << result << std::endl;
            if (kTexture)
                ktxTexture_Destroy(kTexture);
            return false;
        }

        // We will extract first mip level and copy to CPU-side RGBA8 if possible (simple fallback)
        // NOTE: KTX can store GPU-compressed formats — proper workflow is to upload the GPU compressed data.
        // Here we attempt to get the image data as raw bytes if libktx decoded it.
        ktx_size_t dataSize = ktxTexture_GetDataSize(kTexture);
        if (dataSize == 0)
        {
            std::cerr << "libktx: texture has no image data (possibly compressed) - consider using GPU upload path\n";
            ktxTexture_Destroy(kTexture);
            return false;
        }

        // For simplicity, we copy the full blob into a TextureImage (may be compressed!)
        TextureImage img;
        img.path = texturePath;
        img.mipLevels = ktxTexture_GetNumLevels(kTexture);
        img.width = static_cast<uint32_t>(ktxTexture_GetWidth(kTexture, 0));   // level 0 width
        img.height = static_cast<uint32_t>(ktxTexture_GetHeight(kTexture, 0)); // level 0 height

        // If ktxTexture_GetDataSize returns the total bytes, copy them.
        const ktx_uint8_t *data = ktxTexture_GetData(kTexture);
        if (data && dataSize > 0)
        {
            // WARNING: data may be in GPU-friendly compressed format (basis/ASTC/etc).
            // We still store raw bytes; when uploading to GPU you should handle the format accordingly.
            img.pixels.resize(dataSize);
            std::memcpy(img.pixels.data(), data, dataSize);
            // NOTE: img.pixels will contain compressed bytes for many KTX2 files — your Texture->GPU upload must handle that.
        }
        else
        {
            std::cerr << "libktx: no accessible image data\n";
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

    // Fallback to stb_image for common formats (png/jpg)
    int w = 0, h = 0, ch = 0;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &w, &h, &ch, STBI_rgb_alpha);
    if (!pixels)
    {
        std::cerr << "stb_image: failed to load: " << texturePath << std::endl;
        return false;
    }

    TextureImage img;
    img.width = static_cast<uint32_t>(w);
    img.height = static_cast<uint32_t>(h);
    img.mipLevels = 1;
    img.path = texturePath;
    size_t size = static_cast<size_t>(w) * static_cast<size_t>(h) * 4;
    img.pixels.resize(size);
    std::memcpy(img.pixels.data(), pixels, size);
    stbi_image_free(pixels);

    model.textures.push_back(std::move(img));
    outTextureIndex = static_cast<int>(model.textures.size()) - 1;
    return true;
}