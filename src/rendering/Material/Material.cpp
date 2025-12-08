#include "Material.hpp"
#include "../Texture/Texture.hpp"
#include "../../core/Device.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../utils/external/stb_image_write.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <fstream>

// ============================================================================
// Constructeurs
// ============================================================================

Material::Material(Device *device, const std::string &baseDir)
    : m_device(device), m_hasNormalMap(false), m_hasMetallicRoughness(false), m_hasAO(false)
{
    createDefaultTextures();
    loadTexturesFromDirectory(baseDir);
}

Material::Material(Device *device,
                   const std::string &baseColorPath,
                   const std::string &normalPath,
                   const std::string &metallicRoughnessPath,
                   const std::string &aoPath)
    : m_device(device), m_hasNormalMap(false), m_hasMetallicRoughness(false), m_hasAO(false)
{
    createDefaultTextures();
    loadTexturesExplicit(baseColorPath, normalPath, metallicRoughnessPath, aoPath);
}

Material::~Material()
{
    // Les unique_ptr se nettoient automatiquement
}

// ============================================================================
// Getters pour descriptor info
// ============================================================================

VkDescriptorImageInfo Material::getBaseColorInfo() const
{
    return m_baseColorMap->getDescriptorInfo();
}

VkDescriptorImageInfo Material::getNormalInfo() const
{
    if (m_hasNormalMap)
    {
        return m_normalMap->getDescriptorInfo();
    }
    return m_defaultNormal->getDescriptorInfo();
}

VkDescriptorImageInfo Material::getMetallicRoughnessInfo() const
{
    if (m_hasMetallicRoughness)
    {
        return m_metallicRoughnessMap->getDescriptorInfo();
    }
    return m_defaultMetallicRoughness->getDescriptorInfo();
}

VkDescriptorImageInfo Material::getAOInfo() const
{
    if (m_hasAO)
    {
        return m_aoMap->getDescriptorInfo();
    }
    return m_defaultWhite->getDescriptorInfo();
}

// ============================================================================
// Chargement des textures depuis un répertoire
// ============================================================================

void Material::loadTexturesFromDirectory(const std::string &baseDir)
{
    namespace fs = std::filesystem;

    std::cout << "Loading material from directory: " << baseDir << std::endl;

    // Extensions supportées (par ordre de priorité)
    const std::vector<std::string> extensions = {". ktx2", ".png", ".jpg", ".jpeg"};

    // Lambda pour chercher un fichier avec différentes extensions
    auto findTexture = [&](const std::string &baseName) -> std::string
    {
        for (const auto &ext : extensions)
        {
            fs::path texPath = fs::path(baseDir) / (baseName + ext);
            if (fs::exists(texPath))
            {
                return texPath.string();
            }
        }
        return "";
    };

    // 1. Base Color (OBLIGATOIRE)
    std::string baseColorPath = findTexture("baseColor");
    if (baseColorPath.empty())
    {
        baseColorPath = findTexture("albedo"); // Nom alternatif
    }
    if (baseColorPath.empty())
    {
        baseColorPath = findTexture("diffuse"); // Ancien nom
    }

    if (baseColorPath.empty())
    {
        std::cerr << "No base color texture found in " << baseDir << std::endl;
        std::cerr << "      Using white default texture" << std::endl;
        m_baseColorMap = createSolidColorTexture(255, 255, 255, 255, true);
    }
    else
    {
        std::cout << "  ✓ Base Color: " << baseColorPath << std::endl;
        m_baseColorMap = std::make_unique<Texture>(m_device, baseColorPath);
    }

    // 2.  Normal Map (OPTIONNEL)
    std::string normalPath = findTexture("normal");
    if (!normalPath.empty())
    {
        std::cout << "  ✓ Normal Map: " << normalPath << std::endl;
        m_normalMap = std::make_unique<Texture>(m_device, normalPath);
        m_hasNormalMap = true;
    }
    else
    {
        std::cout << "  ⊗ Normal Map: using default (flat)" << std::endl;
        m_hasNormalMap = false;
    }

    // 3. Metallic/Roughness (OPTIONNEL)
    std::string metallicRoughnessPath = findTexture("metallicRoughness");
    if (metallicRoughnessPath.empty())
    {
        metallicRoughnessPath = findTexture("metalRoughness"); // Variante
    }
    if (metallicRoughnessPath.empty())
    {
        metallicRoughnessPath = findTexture("orm"); // OcclusionRoughnessMetallic
    }

    if (!metallicRoughnessPath.empty())
    {
        std::cout << "  ✓ Metallic/Roughness: " << metallicRoughnessPath << std::endl;
        m_metallicRoughnessMap = std::make_unique<Texture>(m_device, metallicRoughnessPath);
        m_hasMetallicRoughness = true;
    }
    else
    {
        std::cout << "  ⊗ Metallic/Roughness: using default (non-metallic, 0. 5 roughness)" << std::endl;
        m_hasMetallicRoughness = false;
    }

    // 4. Ambient Occlusion (OPTIONNEL)
    std::string aoPath = findTexture("ao");
    if (aoPath.empty())
    {
        aoPath = findTexture("ambientOcclusion");
    }
    if (aoPath.empty())
    {
        aoPath = findTexture("occlusion");
    }

    if (!aoPath.empty())
    {
        std::cout << "  ✓ Ambient Occlusion: " << aoPath << std::endl;
        m_aoMap = std::make_unique<Texture>(m_device, aoPath);
        m_hasAO = true;
    }
    else
    {
        std::cout << "  ⊗ Ambient Occlusion: using default (white, no occlusion)" << std::endl;
        m_hasAO = false;
    }

    std::cout << "✓ Material loaded successfully" << std::endl;
}

// ============================================================================
// Chargement avec chemins explicites
// ============================================================================

void Material::loadTexturesExplicit(const std::string &baseColorPath,
                                    const std::string &normalPath,
                                    const std::string &metallicRoughnessPath,
                                    const std::string &aoPath)
{
    std::cout << "  Loading material with explicit paths" << std::endl;

    // 1. Base Color (OBLIGATOIRE)
    if (!baseColorPath.empty() && std::filesystem::exists(baseColorPath))
    {
        std::cout << "  ✓ Base Color: " << baseColorPath << std::endl;
        m_baseColorMap = std::make_unique<Texture>(m_device, baseColorPath);
    }
    else
    {
        std::cerr << "WARNING:   Base color texture not found: " << baseColorPath << std::endl;
        std::cerr << "      Using white default texture" << std::endl;
        m_baseColorMap = createSolidColorTexture(255, 255, 255, 255, true);
    }

    // 2. Normal Map (OPTIONNEL)
    if (!normalPath.empty() && std::filesystem::exists(normalPath))
    {
        std::cout << "  ✓ Normal Map: " << normalPath << std::endl;
        m_normalMap = std::make_unique<Texture>(m_device, normalPath);
        m_hasNormalMap = true;
    }
    else
    {
        std::cout << "  ⊗ Normal Map: using default" << std::endl;
        m_hasNormalMap = false;
    }

    // 3. Metallic/Roughness (OPTIONNEL)
    if (!metallicRoughnessPath.empty() && std::filesystem::exists(metallicRoughnessPath))
    {
        std::cout << "  ✓ Metallic/Roughness: " << metallicRoughnessPath << std::endl;
        m_metallicRoughnessMap = std::make_unique<Texture>(m_device, metallicRoughnessPath);
        m_hasMetallicRoughness = true;
    }
    else
    {
        std::cout << "  ⊗ Metallic/Roughness: using default" << std::endl;
        m_hasMetallicRoughness = false;
    }

    // 4. Ambient Occlusion (OPTIONNEL)
    if (!aoPath.empty() && std::filesystem::exists(aoPath))
    {
        std::cout << "  ✓ Ambient Occlusion: " << aoPath << std::endl;
        m_aoMap = std::make_unique<Texture>(m_device, aoPath);
        m_hasAO = true;
    }
    else
    {
        std::cout << "  ⊗ Ambient Occlusion: using default" << std::endl;
        m_hasAO = false;
    }

    std::cout << "✓ Material loaded successfully" << std::endl;
}

// ============================================================================
// Création des textures par défaut
// ============================================================================

void Material::createDefaultTextures()
{
    // Blanc pur (pour base color et AO par défaut)
    m_defaultWhite = createSolidColorTexture(255, 255, 255, 255, true);

    // Normal map plate : (0.5, 0. 5, 1.0) en RGB = normale pointant vers +Z
    // En RGB 8-bit: (128, 128, 255)
    m_defaultNormal = createSolidColorTexture(128, 128, 255, 255, false);

    // Metallic/Roughness par défaut :
    // R = AO (1.0 = pas d'occlusion) = 255
    // G = Roughness (0.5 = mi-rugueux) = 128
    // B = Metallic (0.0 = diélectrique) = 0
    m_defaultMetallicRoughness = createSolidColorTexture(255, 128, 0, 255, false);

    std::cout << "✓ Default textures created" << std::endl;
}

// ============================================================================
// Création d'une texture de couleur unie (1x1 pixel)
// ============================================================================

std::unique_ptr<Texture> Material::createSolidColorTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool sRGB)
{
    // On va créer un fichier temporaire PNG 1x1 et le charger avec Texture
    // Alternative : modifier Texture pour accepter des données brutes
    // Pour l'instant, on utilise une approche simple avec un fichier temporaire

    namespace fs = std::filesystem;

    // Créer un répertoire temporaire pour les textures par défaut
    fs::path tempDir = fs::temp_directory_path() / "tesseract_default_textures";
    fs::create_directories(tempDir);

    // Nom de fichier basé sur les valeurs RGBA
    std::string filename = "default_" +
                           std::to_string(r) + "_" +
                           std::to_string(g) + "_" +
                           std::to_string(b) + "_" +
                           std::to_string(a) +
                           (sRGB ? "_srgb" : "_linear") +
                           ".temp";

    fs::path texturePath = tempDir / filename;

    // Si le fichier existe déjà, on le charge directement
    if (fs::exists(texturePath))
    {
        return std::make_unique<Texture>(m_device, texturePath.string());
    }

    // Sinon, on crée les données raw et on les écrit
    // Format : un pixel RGBA
    uint8_t pixelData[4] = {r, g, b, a};

    if (!stbi_write_png(texturePath.string().c_str(), 1, 1, 4, pixelData, 4))
    {
        throw std::runtime_error("Failed to create default texture: " + texturePath.string());
    }

    return std::make_unique<Texture>(m_device, texturePath.string());
}