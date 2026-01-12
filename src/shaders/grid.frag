#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

// Paramètres de la grille
const float GRID_STEP = 1.0;      // Taille d'une case
const float GRID_FADE = 64.0;     // Distance de fondu
const float GRID_THICK = 0.5;    // Épaisseur de la ligne
const vec3 GRID_COLOR = vec3(0.5, 0.5, 0.5);
const vec3 AXIS_COLOR = vec3(0.9, 0.2, 0.2);
const float AXIS_THICK = 0.06;

void main() {
    // Position monde sur le plan XZ
    vec2 pos = fragWorldPos.xz;
    float dist = length(ubo.camPos.xz - pos);
    float fade = clamp(1.0 - dist / GRID_FADE, 0.0, 1.0);

    // Grille principale
    float grid = abs(fract(pos.x / GRID_STEP - 0.5) - 0.5) / fwidth(pos.x / GRID_STEP);
    grid = min(grid, abs(fract(pos.y / GRID_STEP - 0.5) - 0.5) / fwidth(pos.y / GRID_STEP));
    float line = 1.0 - smoothstep(0.0, GRID_THICK, grid);

    // Axe X et Z plus épais
    float axisX = 1.0 - smoothstep(0.0, AXIS_THICK, abs(pos.x) / fwidth(pos.x));
    float axisZ = 1.0 - smoothstep(0.0, AXIS_THICK, abs(pos.y) / fwidth(pos.y));
    vec3 color = mix(GRID_COLOR, AXIS_COLOR, max(axisX, axisZ));

    // Mélange grille et axes
    float alpha = max(line, max(axisX, axisZ)) * fade;
    if (alpha < 0.01)
        discard;
    outColor = vec4(color, alpha);
}
