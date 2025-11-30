#include "ShaderLoader.hpp"
#include "FileUtils.hpp"
#include <stdexcept>
#include <iostream>

VkShaderModule ShaderLoader::loadShader(VkDevice device, const std::string& filename) {
    if (!FileUtils::fileExists(filename)) {
        throw std::runtime_error("Shader file not found: " + filename);
    }

    std::vector<char> code = FileUtils::readBinaryFile(filename);

    if (!isValidSpirV(code)) {
        throw std::runtime_error("Invalid SPIR-V file: " + filename);
    }

    std::cout << "✓ Loaded shader: " << filename << " (" << code.size() << " bytes)" << std::endl;

    return createShaderModule(device, code);
}

VkShaderModule ShaderLoader::createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void ShaderLoader::destroyShaderModule(VkDevice device, VkShaderModule shaderModule) {
    if (shaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }
}

bool ShaderLoader::isValidShaderFile(const std::string& filename) {
    if (!FileUtils::fileExists(filename)) {
        return false;
    }

    try {
        std::vector<char> code = FileUtils::readBinaryFile(filename);
        return isValidSpirV(code);
    } catch (...) {
        return false;
    }
}

bool ShaderLoader::isValidSpirV(const std::vector<char>& code) {
    // Le fichier doit faire au moins 4 bytes (pour le magic number)
    if (code.size() < 4) {
        return false;
    }

    // Vérifier le magic number SPIR-V
    uint32_t magicNumber = *reinterpret_cast<const uint32_t*>(code.data());
    return magicNumber == SPIRV_MAGIC_NUMBER;
}