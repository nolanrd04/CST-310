#version 330 core
layout (location = 0) in vec3 aPos;    // Sphere position
layout (location = 1) in vec3 aNormal; // Sphere normal

out vec3 WorldPos;    // World-space position
out vec3 WorldNormal; // World-space normal

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection; // Projection matrix

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0f);       // Object -> world
    WorldPos = worldPos.xyz;                        // Pass world position

    mat3 normalMatrix = transpose(inverse(mat3(model))); // Correct normal transform
    WorldNormal = normalize(normalMatrix * aNormal);     // Pass world normal

    gl_Position = projection * view * worldPos;     // Final clip-space position
}
