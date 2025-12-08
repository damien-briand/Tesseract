#version 450

// Textures PBR
layout(binding = 1) uniform sampler2D baseColorMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D metallicRoughnessMap;
layout(binding = 4) uniform sampler2D aoMap;

// Inputs du vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec3 fragBitangent;
layout(location = 5) in vec3 fragWorldPos;

// Output
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Fonction pour récupérer la normale depuis la normal map
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
    
    // Construire la matrice TBN
    mat3 TBN = mat3(
        normalize(fragTangent),
        normalize(fragBitangent),
        normalize(fragNormal)
    );
    
    return normalize(TBN * tangentNormal);
}

// PBR simplifié - Distribution GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Fonction de Fresnel (Schlick approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

void main() {
    // Échantillonner les textures PBR
    vec3 albedo = pow(texture(baseColorMap, fragTexCoord).rgb, vec3(2.2)); // sRGB to linear
    vec3 normal = getNormalFromMap();
    float metallic = texture(metallicRoughnessMap, fragTexCoord).b;  // Blue channel
    float roughness = texture(metallicRoughnessMap, fragTexCoord).g; // Green channel
    float ao = texture(aoMap, fragTexCoord).r;
    
    // Lumière directionnelle simple (à remplacer par vos vraies lumières)
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 lightColor = vec3(1.0);
    
    vec3 viewDir = normalize(-fragWorldPos); // Camera à l'origine pour l'instant
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Calcul PBR
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, halfwayDir, roughness);   
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);      
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
           
    vec3 numerator = NDF * G * F; 
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(normal, lightDir), 0.0);        
    
    vec3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL;
    
    // Ambient
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;
    
    // Tone mapping et gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
    
    outColor = vec4(color, 1.0);
}