#version 330 core
layout (location = 0) in vec3 aPos; // Receives aPos
layout (location = 1) in vec3 aNormal; // Receives aNormal
layout (location = 2) in vec2 aTexCoord; // Receives aTexCoord
layout (location = 3) in vec3 aTangent; // Receives aTangent
layout (location = 4) in vec3 aBitangent; // Receives aBitangent

out vec3 FragPos; // Returns FragPos
out vec3 Normal; // Returns Normal
out vec2 TexCoord; // Returns TexCoord
out mat3 TBN; // Returns TBN matrix

uniform mat4 model; // Receives model uniform
uniform mat4 view; // Receives view uniform
uniform mat4 projection; // Receives projection uniform

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f); // Implements transformations
    FragPos = vec3(model * vec4(aPos, 1.0)); // Sets fragment position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Normalizes
    TexCoord = aTexCoord; // Set texture coordinate
    
    // Transform tangent and bitangent to world space
    vec3 T = normalize(mat3(model) * aTangent); // Normalize tangent
    vec3 B = normalize(mat3(model) * aBitangent); // Normalize bitangent
    vec3 N = normalize(Normal); // Normalize normal
    
    // Create TBN matrix (transpose for transformation from world space to tangent space)
    TBN = transpose(mat3(T, B, N)); // Create TBN matrix
}
