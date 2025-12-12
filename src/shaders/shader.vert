#version 450

// Uniform Buffer (matrices View et Projection communes)
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

// Push Constant pour la matrice Model de chaque objet
layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

// Inputs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

// Outputs
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec3 fragWorldPos;
layout(location = 6) out vec3 fragViewDir;

void main() {
    // CHANGÉ : Utiliser push. model au lieu de ubo.model
    vec4 worldPos = push.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    
    // Transformer les vecteurs TBN en world space
    mat3 normalMatrix = transpose(inverse(mat3(push.model)));
    
    fragNormal = normalize(normalMatrix * inNormal);
    fragTangent = normalize(normalMatrix * inTangent);
    fragBitangent = normalize(cross(fragNormal, fragTangent));
    
    fragViewDir = normalize(ubo.camPos - fragWorldPos);
}