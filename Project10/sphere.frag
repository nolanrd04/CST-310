#version 330 core
out vec4 FragColor; // Returns FragColor

in vec3 Normal; // Receives normal
in vec3 FragPos; // Receives FragPos
in vec2 TexCoord; // Receives texture coordinate
in mat3 TBN; // Receives TBN matrix
in vec3 TangentViewPos; // Receives tangent space view position
in vec3 TangentFragPos; // Receives tangent space fragment position

uniform vec3 lightPos; // Receives lightPos uniform
uniform vec3 viewPos; // Receives viewPos uniform
uniform vec3 lightColor; // Receives lightColor uniform
uniform sampler2D diffuseTexture; // Receives diffuse texture sampler
uniform sampler2D heightMap; // Receives height map sampler

// Parallax mapping function
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.01f; // Height scale for parallax effect
    float height = texture(heightMap, texCoords).r; // Get height from height map
    vec2 p = viewDir.xy / viewDir.z * (height * heightScale); // Calculate parallax offset
    return texCoords - p; // Return offset texture coordinates
}

void main() {
    // Transform light position and view direction to tangent space
    vec3 tangentLightPos = TBN * lightPos; // Transform light position to tangent space
    
    // Get view direction in tangent space
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos); // Get view direction
    viewDir.z = max(viewDir.z, 0.1f); // Clamp z component to avoid division issues
    
    // Apply parallax mapping
    vec2 parallaxTexCoord = ParallaxMapping(TexCoord, viewDir); // Get parallax-adjusted texture coordinates
    
    // Clamp texture coordinates to 0-1 range
    if(parallaxTexCoord.x > 1.0 || parallaxTexCoord.y > 1.0 || parallaxTexCoord.x < 0.0 || parallaxTexCoord.y < 0.0) // Check bounds
        discard; // Discard out of bounds pixels
    
    // Sample the diffuse texture
    vec3 diffuse_color = texture(diffuseTexture, parallaxTexCoord).rgb; // Sample diffuse texture
    
    // Sample normal from geometry
    vec3 norm = normalize(-Normal); // Normalize normal (inverted for correct orientation)
    
    // Calculate lighting in tangent space
    vec3 tangentNormal = norm; // Use the computed normal
    vec3 lightDir = normalize(tangentLightPos - TangentFragPos); // Get light direction
    
    // ambient
    float ambientStrength = 0.5; // Set ambient strength
    vec3 ambient = ambientStrength * lightColor; // Sets ambient
    
    // diffuse
    float diff = max(dot(tangentNormal, lightDir), 0.0); // Gets diff
    vec3 diffuse = diff * lightColor; // Sets diffuse

    // specular
    float specularStrength = 0.25f; // Sets specularStrength
    vec3 reflectDir = reflect(-lightDir, tangentNormal); // Get reflectDir
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0); // Get spec
    vec3 specular = specularStrength * spec * lightColor; // Set specular

    vec3 result = (ambient + diffuse + specular) * diffuse_color; // Calculate result
    FragColor = vec4(result, 1.0f); // Set FragColor output
}
