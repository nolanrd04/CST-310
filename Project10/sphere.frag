#version 330 core
out vec4 FragColor; // Final sphere color

in vec3 WorldPos;    // World-space position
in vec3 WorldNormal; // World-space normal

uniform vec3 viewPos;       // Camera position in world space
uniform samplerCube skybox; // Cubemap for environment mapping

void main() {
    vec3 I = normalize(WorldPos - viewPos);            // View vector from camera to fragment
    vec3 N = normalize(WorldNormal);                   // Outward normal

    vec3 lookupDir;
    if (gl_FrontFacing) {
        lookupDir = reflect(I, N);                     // Outside sphere: reflective environment mapping
    } else {
        lookupDir = I;                                 // Inside sphere: direct cubemap direction (upright interior)
    }

    FragColor = texture(skybox, lookupDir);            // Sample cubemap
}
