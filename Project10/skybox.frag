#version 330 core
out vec4 FragColor;

in vec3 TexCoords; // Direction vector from vertex shader

uniform samplerCube skybox; // Cubemap sampler

void main() {
    FragColor = texture(skybox, TexCoords); // Sample the cubemap using the direction vector
}