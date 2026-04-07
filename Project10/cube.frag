#version 330 core
out vec4 FragColor; // Returns FragColor

in vec3 Normal; // Receives Normal
in vec3 FragPos; // Receives FragPos
in vec2 TexCoord; // Receives texture coordinates
in mat3 TBN; // Receives TBN matrix for tangent space
in vec3 TangentViewPos; // Camera position in tangent space
in vec3 TangentFragPos; // Fragment position in tangent space

uniform vec3 lightPos; // Uniform loc for lightPos vec3
uniform vec3 viewPos; // Uniform loc for viewPos vec3
uniform vec3 lightColor; // Uniform loc for lightColor vec3
uniform vec3 cubeColor; // Uniform loc for cubeColor vec3

// Textures
uniform sampler2D diffuseTexture; // Diffuse color texture
uniform sampler2D heightMap; // Height map for parallax mapping

// Parallax mapping function
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    float heightScale = 0.01; // Very subtle parallax effect to avoid zoom
    float height = texture(heightMap, texCoords).r;
    height = height * 2.0 - 1.0; // Remap from [0,1] to [-1,1]
    
    // Avoid extreme offsets - clamp the view direction Z
    float viewZ = max(viewDir.z, 0.1);
    vec2 p = viewDir.xy / viewZ * (height * heightScale);
    return texCoords - p;
}

void main() {
    // Use original texture coordinates without parallax for now
    vec3 diffuse_color = texture(diffuseTexture, TexCoord).rgb;
    
    // Recalculate light direction in world space for basic lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;
    
    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.25;
    vec3 viewDir_world = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir_world, reflectDir), 0.0), 8.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * diffuse_color;
    FragColor = vec4(result, 1.0);
}
