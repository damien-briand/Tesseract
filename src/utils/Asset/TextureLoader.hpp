#pragma once
#include <string>
#include "Model.hpp"

class TextureLoader
{
public:
    // charge image sur CPU dans Model::textures[index]; supporte .ktx2 si libktx est disponible
    static bool loadTextureToModel(const std::string &texturePath, Model &model, int &outTextureIndex);
};