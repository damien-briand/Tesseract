#version 450

layout(binding = 1) uniform sampler2D texSampler;

// Inputs du vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord; 

// Output
layout(location = 0) out vec4 outColor;

void main() {
    // outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0);
    outColor = texture(texSampler, fragTexCoord);
}