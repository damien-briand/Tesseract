#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;      // NOUVEAU
layout(location = 4) in vec3 inTangent;     // NOUVEAU pour normal mapping

// Outputs
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;   // NOUVEAU
layout(location = 3) out vec3 fragTangent;  // NOUVEAU
layout(location = 4) out vec3 fragBitangent;// NOUVEAU
layout(location = 5) out vec3 fragWorldPos; // NOUVEAU pour lighting

void main() {
    vec4 worldPos = ubo. model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragWorldPos = worldPos.xyz;
    
    // Transformer les vecteurs TBN dans l'espace world
    mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
    fragNormal = normalize(normalMatrix * inNormal);
    fragTangent = normalize(normalMatrix * inTangent);
    fragBitangent = cross(fragNormal, fragTangent);
}