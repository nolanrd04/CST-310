#version 330 core
layout (location = 0) in vec3 aPos;      // Cube position
layout (location = 2) in vec2 aTexCoord; // Cube UV

out vec3 FragPos; // World-space fragment position
out vec2 TexCoord; // UV for parallax + bump sampling

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection; // Projection matrix

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0f);        // World-space position
    FragPos = worldPos.xyz;                          // Pass world position
    TexCoord = vec2(aTexCoord.x, 1.0f - aTexCoord.y); // Flip V to correct SOIL image orientation
    gl_Position = projection * view * worldPos;      // Final clip-space position
}
