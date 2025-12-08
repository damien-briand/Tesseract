#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>

class Device;
class Texture;

/**
 * @brief Gère un ensemble de textures PBR pour un matériau
 * 
 * Cette classe encapsule toutes les textures nécessaires pour le rendu PBR :
 * - Base Color (Albedo) : Couleur de base du matériau
 * - Normal Map : Détails de surface en perturbant les normales
 * - Metallic/Roughness : Propriétés physiques du matériau (canal B = metallic, canal G = roughness)
 * - Ambient Occlusion : Simule les zones d'ombre ambiante
 * 
 * Si une texture est absente, une texture par défaut est utilisée.
 */
class Material {
public:
    /**
     * @brief Constructeur - charge les textures PBR depuis un répertoire
     * 
     * Convention de nommage attendue dans baseDir :
     * - baseColor.png (ou . jpg, .ktx2)
     * - normal.png
     * - metallicRoughness.png (metallic en B, roughness en G)
     * - ao.png (ambient occlusion)
     * 
     * @param device Pointeur vers le device Vulkan
     * @param baseDir Chemin du répertoire contenant les textures
     */
    Material(Device* device, const std::string& baseDir);
    
    /**
     * @brief Constructeur avec chemins de textures explicites
     * 
     * @param device Pointeur vers le device Vulkan
     * @param baseColorPath Chemin vers la texture base color
     * @param normalPath Chemin vers la normal map (vide = texture par défaut)
     * @param metallicRoughnessPath Chemin vers metallic/roughness (vide = texture par défaut)
     * @param aoPath Chemin vers ambient occlusion (vide = texture par défaut)
     */
    Material(Device* device, 
             const std::string& baseColorPath,
             const std::string& normalPath,
             const std::string& metallicRoughnessPath,
             const std::string& aoPath);
    
    ~Material();

    // Suppression de la copie
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Getters pour les descriptor image info (utilisés par DescriptorPool)
    VkDescriptorImageInfo getBaseColorInfo() const;
    VkDescriptorImageInfo getNormalInfo() const;
    VkDescriptorImageInfo getMetallicRoughnessInfo() const;
    VkDescriptorImageInfo getAOInfo() const;

    // Vérifications
    bool hasNormalMap() const { return m_hasNormalMap; }
    bool hasMetallicRoughness() const { return m_hasMetallicRoughness; }
    bool hasAO() const { return m_hasAO; }

    // Getters directs (optionnel, si besoin d'accéder aux textures directement)
    const Texture* getBaseColorTexture() const { return m_baseColorMap.get(); }
    const Texture* getNormalTexture() const { return m_normalMap.get(); }
    const Texture* getMetallicRoughnessTexture() const { return m_metallicRoughnessMap. get(); }
    const Texture* getAOTexture() const { return m_aoMap.get(); }

private:
    Device* m_device;
    
    // Textures PBR
    std::unique_ptr<Texture> m_baseColorMap;        // Albedo/Diffuse (obligatoire)
    std::unique_ptr<Texture> m_normalMap;           // Normal map (optionnel)
    std::unique_ptr<Texture> m_metallicRoughnessMap; // Metallic (B) + Roughness (G) (optionnel)
    std::unique_ptr<Texture> m_aoMap;               // Ambient Occlusion (optionnel)
    
    // Textures par défaut (1x1 pixel) utilisées si les textures sont absentes
    std::unique_ptr<Texture> m_defaultWhite;      // Texture blanche (1,1,1,1)
    std::unique_ptr<Texture> m_defaultNormal;     // Normal map plate (0. 5, 0.5, 1.0)
    std::unique_ptr<Texture> m_defaultMetallicRoughness; // Non-métallique, mi-rugueux (0, 0. 5, 0)
    
    // Flags pour savoir si les textures optionnelles sont présentes
    bool m_hasNormalMap;
    bool m_hasMetallicRoughness;
    bool m_hasAO;
    
    // Méthodes privées
    void loadTexturesFromDirectory(const std::string& baseDir);
    void loadTexturesExplicit(const std::string& baseColorPath,
                             const std::string& normalPath,
                             const std::string& metallicRoughnessPath,
                             const std::string& aoPath);
    void createDefaultTextures();
    
    /**
     * @brief Crée une texture 1x1 avec une couleur spécifique
     * @param r,g,b,a Composantes de couleur (0-255)
     * @param sRGB Si true, utilise un format SRGB, sinon linear
     * @return Pointeur unique vers la texture créée
     */
    std::unique_ptr<Texture> createSolidColorTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool sRGB);
};