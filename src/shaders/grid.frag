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
const float GRID_STEP = 2.0;
const float GRID_FADE = 64.0;
const float GRID_THICK = 0.5;
const vec3 GRID_COLOR = vec3(0.3, 0.3, 0.3);

// Couleurs des axes
const vec3 AXIS_X_COLOR = vec3(1.0, 0.2, 0.2);  // Rouge pour X
const vec3 AXIS_Z_COLOR = vec3(0.2, 0.2, 1.0);  // Bleu pour Z
const float AXIS_THICK = 0.01;

void main() {
    vec2 pos = fragWorldPos.xz;
    float dist = length(ubo.camPos.xz - pos);
    float fade = clamp(1.0 - dist / GRID_FADE, 0.0, 1.0);
    
    // Grille principale
    vec2 coord = pos / GRID_STEP;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    float gridAlpha = 1.0 - smoothstep(0.0, GRID_THICK, line);
    
    // Axes (avec épaisseur en unités monde)
    float axisX = smoothstep(AXIS_THICK, 0.0, abs(pos.y));  // Axe X (ligne Z=0)
    float axisZ = smoothstep(AXIS_THICK, 0.0, abs(pos.x));  // Axe Z (ligne X=0)
    
    // Couleur finale
    vec3 color = GRID_COLOR;
    float alpha = gridAlpha;
    
    // Surcharge avec les axes colorés
    if (axisX > 0.0) {
        color = AXIS_X_COLOR;
        alpha = max(alpha, axisX);
    }
    if (axisZ > 0.0) {
        color = AXIS_Z_COLOR;
        alpha = max(alpha, axisZ);
    }
    
    alpha *= fade;
    
    if (alpha < 0.01)
        discard;
    
    outColor = vec4(color, alpha);
}