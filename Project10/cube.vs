#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords; // Cubemap direction vector (vertex position)
out vec3 FragPos;   // World space position

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection; // Projection matrix

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aPos;                          // Vertex position is the cubemap lookup direction
    FragPos = vec3(model * vec4(aPos, 1.0));   // World position
}
