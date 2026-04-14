#version 330 core
layout (location = 0) in vec3 aPos; // Receives aPos

out vec3 TexCoords; // Cubemap direction vector (vertex position used as direction)
out vec3 FragPos;   // World space position for lighting

uniform mat4 model;      // Receives model uniform
uniform mat4 view;       // Receives view uniform
uniform mat4 projection; // Receives projection uniform

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0f);
    TexCoords = aPos; // Vertex position is the cubemap lookup direction
    FragPos = vec3(model * vec4(aPos, 1.0));   // World position for lighting
}
