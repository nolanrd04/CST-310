#version 330 core
out vec4 FragColor;

in vec3 TexCoords; // Cubemap direction vector from vertex shader
in vec3 FragPos;   // World space position

uniform samplerCube skybox; // Cubemap texture (6 face images)

void main() {
    // Sample the cubemap using the direction vector
    FragColor = texture(skybox, vec3(TexCoords.x, -TexCoords.y, TexCoords.z));
}
