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
    mat3 normalMatrix = transpose(inverse(mat3(model))); // Normal matrix for non-uniform scale
    Normal = normalMatrix * aNormal; // Transform normal to world space
    TexCoord = aTexCoord; // Set texture coordinate
    
    // Transform tangent and bitangent to world space
    vec3 T = normalize(normalMatrix * aTangent); // Transform tangent
    vec3 Binput = normalize(normalMatrix * aBitangent); // Transform provided bitangent
    vec3 N = normalize(Normal); // Normalize transformed normal
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt to keep T orthogonal to N
    float handedness = (dot(cross(N, T), Binput) < 0.0) ? -1.0 : 1.0; // Preserve handedness
    vec3 B = normalize(cross(N, T)) * handedness; // Rebuild orthonormal bitangent
    
    // Create TBN matrix (transpose for transformation from world space to tangent space)
    TBN = transpose(mat3(T, B, N)); // Create TBN matrix
}
