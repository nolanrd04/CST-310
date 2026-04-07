#version 330 core
layout (location = 0) in vec3 aPos; // Receives aPos
layout (location = 1) in vec3 aColor; // Color (unused but kept for vertex structure)
layout (location = 2) in vec2 aTexCoord; // Receives texture coordinates
layout (location = 3) in vec3 aTangent; // Receives tangent vector
layout (location = 4) in vec3 aBitangent; // Receives bitangent vector

out vec3 FragPos; // Returns FragPos
out vec3 Normal; // Returns Normal
out vec2 TexCoord; // Returns TexCoord
out mat3 TBN; // Returns TBN matrix for tangent space
out vec3 TangentViewPos; // Camera position in tangent space
out vec3 TangentFragPos; // Fragment position in tangent space

uniform mat4 model; // Receives model uniform
uniform mat4 view; // Receives view uniform
uniform mat4 projection; // Receives projection uniform
uniform vec3 viewPos; // Camera position

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0f);
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoord;
    
    // Transform tangent and bitangent to world space
    vec3 Tangent = normalize(mat3(model) * aTangent);
    vec3 Bitangent = normalize(mat3(model) * aBitangent);
    
    // Compute normal as cross product of tangent and bitangent
    vec3 Normal_out = cross(Bitangent, Tangent);
    Normal = Normal_out;
    
    // Create TBN matrix (transpose for world-to-tangent space transformation)
    TBN = transpose(mat3(Tangent, Bitangent, Normal_out));
    
    // Transform positions and camera position to tangent space
    TangentFragPos = TBN * FragPos;
    TangentViewPos = TBN * viewPos;
}
