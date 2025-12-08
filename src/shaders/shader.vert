#version 450

// ============================================================================
// UNIFORM BUFFER - Matrices de transformation (Binding 0)
// ============================================================================
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;      // Transformation modèle -> monde
    mat4 view;       // Transformation monde -> caméra
    mat4 proj;       // Projection perspective
    vec3 camPos;     // Position de la caméra (pour les calculs de réflexion)
} ubo;

// ============================================================================
// VERTEX INPUTS - Données depuis le Vertex Buffer
// ============================================================================
layout(location = 0) in vec3 inPosition;   // Position du vertex
layout(location = 1) in vec3 inColor;      // Couleur du vertex
layout(location = 2) in vec2 inTexCoord;   // Coordonnées de texture (UV)
layout(location = 3) in vec3 inNormal;     // Normale du vertex
layout(location = 4) in vec3 inTangent;    // Tangente du vertex (pour normal mapping)

// ============================================================================
// OUTPUTS vers le Fragment Shader
// ============================================================================
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;      // Normale en espace monde
layout(location = 3) out vec3 fragTangent;     // Tangente en espace monde
layout(location = 4) out vec3 fragBitangent;   // Bitangente en espace monde
layout(location = 5) out vec3 fragWorldPos;    // Position en espace monde
layout(location = 6) out vec3 fragViewDir;     // Direction de vue (pour spéculaire)

// ============================================================================
// MAIN
// ============================================================================
void main() {
    // 1. Calculer la position en espace monde
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    // 2.  Position finale (clip space)
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    // 3. Passer les données de base
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    
    // 4. Transformer les vecteurs TBN (Tangent-Bitangent-Normal) en espace monde
    // La matrice normale ne doit PAS inclure la translation, seulement rotation/scale
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    
    // Normal transformée
    fragNormal = normalize(normalMatrix * inNormal);
    
    // Tangent transformée
    fragTangent = normalize(normalMatrix * inTangent);
    
    // Bitangent calculée (perpendiculaire à Normal et Tangent)
    // Note : On utilise le produit vectoriel pour garantir l'orthogonalité
    fragBitangent = normalize(cross(fragNormal, fragTangent));
    
    // 5. Direction de vue (de la surface vers la caméra)
    fragViewDir = normalize(ubo.camPos - fragWorldPos);
}