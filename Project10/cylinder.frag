#version 330 core
out vec4 FragColor; // Returns FragColor

in vec3 FragPos; // Receives FragPos
in vec2 TexCoord; // Receives texture coordinate
in mat3 TBN; // Receives TBN matrix
  
uniform vec3 lightPos; // Receives lightPos uniform
uniform vec3 viewPos; // Receives viewPos uniform
uniform vec3 lightColor; // Receives lightColor uniform
uniform sampler2D diffuseTexture; // Receives diffuse texture sampler
uniform sampler2D heightMap; // Receives height map sampler
uniform vec2 uvScale; // UV tiling amount for texture repeat

vec3 BumpNormalFromHeight(vec2 texCoords)
{
    vec2 texel = 1.0 / vec2(textureSize(heightMap, 0)); // Texture-space step
    float hL = texture(heightMap, texCoords - vec2(texel.x, 0.0)).r; // Left sample
    float hR = texture(heightMap, texCoords + vec2(texel.x, 0.0)).r; // Right sample
    float hD = texture(heightMap, texCoords - vec2(0.0, texel.y)).r; // Down sample
    float hU = texture(heightMap, texCoords + vec2(0.0, texel.y)).r; // Up sample

    float bumpScale = 5.0; // Stronger bump intensity (more obvious relief)
    float dHdU = (hR - hL) * bumpScale; // Height derivative along U
    float dHdV = (hU - hD) * bumpScale; // Height derivative along V

    return normalize(vec3(-dHdU, -dHdV, 1.0)); // Tangent-space bumped normal
}

void main()
{
    vec2 tiledTexCoord = TexCoord * uvScale; // Repeat UVs across cylinder

    // Transform positions from world space to tangent space
    vec3 tangentFragPos = TBN * FragPos; // Tangent-space fragment position
    vec3 tangentLightPos = TBN * lightPos; // Tangent-space light position
    vec3 tangentViewPos = TBN * viewPos; // Tangent-space view position

    // Build bumped normal from the height map (bump mapping)
    vec3 tangentNormal = BumpNormalFromHeight(tiledTexCoord); // Perturbed tangent-space normal

    vec3 lightDir = normalize(tangentLightPos - tangentFragPos); // Tangent-space light direction
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos); // Tangent-space view direction

    // Sample height map and convert to grayscale luminance
    vec3 mapSample = texture(heightMap, tiledTexCoord).rgb; // Sample texture
    float luminance = dot(mapSample, vec3(0.299, 0.587, 0.114)); // Calculate luminance

    // ambient
    float ambientStrength = 2.0; // Set ambient strength
    vec3 ambient = ambientStrength * lightColor; // Sets ambient

    // diffuse
    float diff = max(dot(tangentNormal, lightDir), 0.0); // Diffuse contribution
    vec3 diffuse = diff * lightColor; // Sets diffuse

    // specular
    float specularStrength = 0.25f; // Sets specular strength
    vec3 reflectDir = reflect(-lightDir, tangentNormal); // Sets reflect direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0); // Sets specular based on power, max, and dot product
    vec3 specular = specularStrength * spec * lightColor; // Sets specular

    vec3 result = (ambient + diffuse + specular) * luminance; // Multiply by grayscale luminance to get colorless texture
    FragColor = vec4(result, 1.0f); // Sets vec4 based on result
} 
