#version 330 core
layout (location = 0) in vec3 aPos;      // Sphere position
layout (location = 1) in vec3 aNormal;   // Sphere normal
layout (location = 2) in vec2 aTexCoord; // Texture coordinate
layout (location = 3) in vec3 aTangent;  // Tangent vector
layout (location = 4) in vec3 aBitangent; // Bitangent vector

out vec3 FragPos;   // World-space position
out vec2 TexCoord;  // Texture coordinate
out mat3 TBN;       // Tangent-Bitangent-Normal matrix

uniform mat4 model;       // Model matrix
uniform mat4 view;        // View matrix
uniform mat4 projection;  // Projection matrix

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0f);        // Object -> world
    FragPos = worldPos.xyz;                          // Pass world position
    TexCoord = aTexCoord;                            // Pass texture coordinate

    // Build TBN matrix for tangent space transformations
    mat3 normalMatrix = transpose(inverse(mat3(model))); // Correct normal transform
    vec3 T = normalize(normalMatrix * aTangent);         // World-space tangent
    vec3 B = normalize(normalMatrix * aBitangent);       // World-space bitangent
    vec3 N = normalize(normalMatrix * aNormal);          // World-space normal
    TBN = mat3(T, B, N);                                 // Build TBN matrix

    gl_Position = projection * view * worldPos;      // Final clip-space position
}
