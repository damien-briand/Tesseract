#version 450

// Input depuis le vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragWorldPos;
layout(location = 6) in vec3 fragViewDir;

// Output
layout(location = 0) out vec4 outColor;

// Uniform Buffer
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    int lightCount;              // Nombre de lumières
    vec3 ambientColor;           // Couleur ambiante
    float ambientIntensity;      // Intensité ambiante
    float exposure;
    float scaleIBLAmbient;
} ubo;

// Storage Buffer pour les lumières
struct LightData {
    vec4 position;              // xyz = position ou direction, w = type indicator
    vec4 color;                 // rgb = color * intensity
    mat4 lightSpaceMatrix;      // Pour shadow mapping (future)
    int lightType;              // 0=Point, 1=Directional, 2=Spot
    float range;
    float innerConeAngle;
    float outerConeAngle;
};

layout(std140, binding = 1) readonly buffer LightBuffer {
    LightData lights[];
} lightBuffer;

// Textures du material
layout(binding = 2) uniform sampler2D texColor;
layout(binding = 3) uniform sampler2D texNormal;
layout(binding = 4) uniform sampler2D texMetallic;

// Push constants pour la matrice model
layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

// Constantes
const float PI = 3.14159265359;

// ============================================================================
// PBR Functions
// ============================================================================

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============================================================================
// Main Fragment Shader
// ============================================================================

void main() {
    // Sample textures
    vec4 albedoSample = texture(texColor, fragTexCoord);
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2));  // sRGB to linear

    vec3 normalMap = texture(texNormal, fragTexCoord).rgb;
    normalMap = normalMap * 2.0 - 1.0;  // [0,1] to [-1,1]

    vec3 metallicRoughness = texture(texMetallic, fragTexCoord).rgb;
    float metallic = metallicRoughness.b;    // Blue channel
    float roughness = metallicRoughness.g;   // Green channel
    float ao = metallicRoughness.r;          // Red channel (AO)

    // Reconstruct TBN matrix
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);
    vec3 N = normalize(fragNormal);
    mat3 TBN = mat3(T, B, N);

    // Apply normal map
    vec3 normal = normalize(TBN * normalMap);

    // View direction
    vec3 V = normalize(fragViewDir);

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);  // Dielectric base reflectivity
    F0 = mix(F0, albedo, metallic);

    // Lighting accumulation
    vec3 Lo = vec3(0.0);

    // Iterate over all lights
    for (int i = 0; i < ubo.lightCount; i++) {
        LightData light = lightBuffer.lights[i];

        vec3 L;
        vec3 radiance;
        float attenuation = 1.0;

        // Calculate light direction and radiance based on type
        if (light.lightType == 1) {
            // Directional light
            L = normalize(-light.position.xyz);  // Direction stored in position
            radiance = light.color.rgb;
        } else if (light.lightType == 0) {
            // Point light
            vec3 lightVec = light.position.xyz - fragWorldPos;
            float distance = length(lightVec);
            L = normalize(lightVec);

            // Attenuation
            attenuation = 1.0 / (distance * distance);
            
            // Range cutoff
            if (distance > light.range) {
                attenuation = 0.0;
            }

            radiance = light.color.rgb * attenuation;
        } else if (light.lightType == 2) {
            // Spot light
            vec3 lightVec = light.position.xyz - fragWorldPos;
            float distance = length(lightVec);
            L = normalize(lightVec);

            // Attenuation
            attenuation = 1.0 / (distance * distance);

            // Spot cone
            vec3 spotDir = normalize(light.position.xyz);  // Stored in position for spots
            float theta = dot(L, -spotDir);
            float epsilon = light.innerConeAngle - light.outerConeAngle;
            float intensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);

            attenuation *= intensity;

            radiance = light.color. rgb * attenuation;
        }

        // Skip if light contribution is negligible
        if (attenuation < 0.001) {
            continue;
        }

        // Calculate per-light radiance
        vec3 H = normalize(V + L);
        float NdotL = max(dot(normal, L), 0.0);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(normal, H, roughness);
        float G = GeometrySmith(normal, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, V), 0.0) * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        // Energy conservation
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        // Add to outgoing radiance
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting
    vec3 ambient = ubo.ambientColor * ubo.ambientIntensity * albedo * ao;

    // Final color
    vec3 color = ambient + Lo;

    // HDR tonemapping (simple Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, albedoSample.a);
}