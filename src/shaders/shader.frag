#version 450

// ============================================================================
// TEXTURES PBR (Bindings 1-4)
// ============================================================================
layout(binding = 1) uniform sampler2D baseColorMap;         // Albedo/Couleur de base
layout(binding = 2) uniform sampler2D normalMap;            // Normal map (tangent space)
layout(binding = 3) uniform sampler2D metallicRoughnessMap; // Metallic (B) + Roughness (G)
layout(binding = 4) uniform sampler2D aoMap;                // Ambient Occlusion

// ============================================================================
// INPUTS depuis le Vertex Shader
// ============================================================================
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragWorldPos;
layout(location = 6) in vec3 fragViewDir;

// ============================================================================
// OUTPUT - Couleur finale
// ============================================================================
layout(location = 0) out vec4 outColor;

// ============================================================================
// CONSTANTES
// ============================================================================
const float PI = 3.14159265359;

// Configuration de la lumière directionnelle (soleil)
// TODO : Passer ces valeurs via un uniform buffer
const vec3 LIGHT_DIR = normalize(vec3(0.5, 1.0, 0.3));  // Direction de la lumière
const vec3 LIGHT_COLOR = vec3(1.0, 0.95, 0.9);          // Couleur légèrement chaude
const float LIGHT_INTENSITY = 4.0;                       // Intensité

// ============================================================================
// FONCTION : Récupérer la normale depuis la normal map
// ============================================================================
// Les normal maps stockent les normales en "tangent space" (espace local de la surface)
// On doit les transformer en "world space" avec la matrice TBN
vec3 getNormalFromMap() {
    // 1. Échantillonner la normal map (valeurs RGB entre 0 et 1)
    vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz;
    
    // 2.  Convertir de [0,1] à [-1,1]
    tangentNormal = tangentNormal * 2.0 - 1.0;
    
    // 3.  Construire la matrice TBN (Tangent-Bitangent-Normal)
    // Cette matrice transforme de tangent space -> world space
    mat3 TBN = mat3(
        normalize(fragTangent),
        normalize(fragBitangent),
        normalize(fragNormal)
    );
    
    // 4. Transformer la normale en world space
    return normalize(TBN * tangentNormal);
}

// ============================================================================
// FONCTION : Distribution Normal (GGX/Trowbridge-Reitz)
// ============================================================================
// Décrit comment les microfacettes sont distribuées
// Plus la roughness est élevée, plus les microfacettes sont désordonnées
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / max(denom, 0.0000001); // Éviter division par zéro
}

// ============================================================================
// FONCTION : Fresnel (Schlick Approximation)
// ============================================================================
// Décrit comment la lumière est réfléchie vs réfractée selon l'angle de vue
// F0 = réflectance à angle normal (0° = vue perpendiculaire)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============================================================================
// FONCTION : Geometry (Smith's Schlick-GGX)
// ============================================================================
// Décrit l'auto-ombrage des microfacettes (masking & shadowing)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / max(denom, 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// ============================================================================
// MAIN - Calcul du rendu PBR
// ============================================================================
void main() {
    // ------------------------------------------------------------------------
    // 1.  ÉCHANTILLONNER LES TEXTURES
    // ------------------------------------------------------------------------
    
    // Base Color (albedo) - convertir de sRGB à linear
    vec3 albedo = pow(texture(baseColorMap, fragTexCoord).rgb, vec3(2.2));
    
    // Normal depuis la normal map
    vec3 N = getNormalFromMap();
    
    // Metallic et Roughness depuis la texture metallicRoughness
    // Convention glTF 2.0 : 
    //   - Canal B (blue) = Metallic
    //   - Canal G (green) = Roughness
    vec3 mrSample = texture(metallicRoughnessMap, fragTexCoord). rgb;
    float metallic = mrSample.b;
    float roughness = mrSample.g;
    
    // Ambient Occlusion
    float ao = texture(aoMap, fragTexCoord).r;
    
    // ------------------------------------------------------------------------
    // 2.  PRÉPARER LES VECTEURS
    // ------------------------------------------------------------------------
    
    vec3 V = normalize(fragViewDir);           // Direction vue (vers caméra)
    vec3 L = normalize(LIGHT_DIR);             // Direction lumière
    vec3 H = normalize(V + L);                 // Halfway vector (entre V et L)
    
    // ------------------------------------------------------------------------
    // 3.  CALCULER F0 (réflectance à angle normal)
    // ------------------------------------------------------------------------
    
    // Pour les diélectriques (non-métaux), F0 ≈ 0.04
    // Pour les métaux, F0 = albedo color
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // ------------------------------------------------------------------------
    // 4.  BRDF - COOK-TORRANCE
    // ------------------------------------------------------------------------
    
    // Calcul des termes de la BRDF
    float NDF = DistributionGGX(N, H, roughness);        // Normal Distribution
    float G = GeometrySmith(N, V, L, roughness);         // Geometry
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);    // Fresnel
    
    // Cook-Torrance BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // ------------------------------------------------------------------------
    // 5.  SÉPARER RÉFLEXION ET RÉFRACTION (kS et kD)
    // ------------------------------------------------------------------------
    
    // kS = énergie réfléchie (spéculaire)
    vec3 kS = F;
    
    // kD = énergie réfractée (diffuse)
    // L'énergie ne peut pas être créée, donc kD = 1 - kS
    vec3 kD = vec3(1.0) - kS;
    
    // Les métaux n'ont pas de diffusion (toute l'énergie est réfléchie)
    kD *= 1.0 - metallic;
    
    // ------------------------------------------------------------------------
    // 6.  CALCULER LA RADIANCE SORTANTE (Lo)
    // ------------------------------------------------------------------------
    
    float NdotL = max(dot(N, L), 0.0);
    
    // Radiance de la lumière directionnelle
    vec3 radiance = LIGHT_COLOR * LIGHT_INTENSITY;
    
    // Équation de rendu finale pour cette lumière
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    
    // ------------------------------------------------------------------------
    // 7.  LUMIÈRE AMBIANTE
    // ------------------------------------------------------------------------
    
    // Ambient très simple (à améliorer avec IBL plus tard)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    // ------------------------------------------------------------------------
    // 8.  COULEUR FINALE
    // ------------------------------------------------------------------------
    
    vec3 color = ambient + Lo;
    
    // Tone mapping (Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction (linear -> sRGB)
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}