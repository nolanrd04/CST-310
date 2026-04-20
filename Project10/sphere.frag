#version 330 core
out vec4 FragColor; // Final color output

in vec3 FragPos;    // World-space position
in vec2 TexCoord;   // Texture coordinate
in mat3 TBN;        // Tangent-Bitangent-Normal matrix

uniform vec3 lightPos;       // Light position in world space
uniform vec3 viewPos;        // Camera position in world space
uniform vec3 lightColor;     // Light color
uniform sampler2D diffuseTexture; // Diffuse texture sampler
uniform sampler2D heightMap;      // Height map sampler
uniform vec2 uvScale;        // UV tiling amount

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float heightScale = 0.05; // Control height offset strength
    float height = texture(heightMap, texCoords).r; // Sample height
    vec2 p = viewDir.xy / max(viewDir.z, 0.1) * (height * heightScale); // Parallax offset
    return texCoords - p;
}

vec3 BumpNormalFromHeight(vec2 texCoords)
{
    vec2 texel = 1.0 / vec2(textureSize(heightMap, 0)); // Texture-space step
    float hL = texture(heightMap, texCoords - vec2(texel.x, 0.0)).r; // Left sample
    float hR = texture(heightMap, texCoords + vec2(texel.x, 0.0)).r; // Right sample
    float hD = texture(heightMap, texCoords - vec2(0.0, texel.y)).r; // Down sample
    float hU = texture(heightMap, texCoords + vec2(0.0, texel.y)).r; // Up sample

    float bumpScale = 5.0; // Bump intensity
    float dHdU = (hR - hL) * bumpScale; // Height derivative along U
    float dHdV = (hU - hD) * bumpScale; // Height derivative along V

    return normalize(vec3(-dHdU, -dHdV, 1.0)); // Tangent-space bumped normal
}

void main()
{
    vec2 tiledTexCoord = TexCoord * uvScale; // Repeat UVs across sphere

    // Transform positions from world space to tangent space
    vec3 tangentFragPos = TBN * FragPos;     // Tangent-space fragment position
    vec3 tangentViewPos = TBN * viewPos;     // Tangent-space view position
    vec3 tangentLightPos = TBN * lightPos;   // Tangent-space light position

    vec3 viewDir = normalize(tangentViewPos - tangentFragPos); // Tangent-space view direction

    // Apply parallax mapping to offset texture coordinates
    vec2 parallaxTexCoord = ParallaxMapping(tiledTexCoord, viewDir);

    // Build bumped normal from the parallax-offset height map
    vec3 tangentNormal = BumpNormalFromHeight(parallaxTexCoord);

    vec3 lightDir = normalize(tangentLightPos - tangentFragPos); // Tangent-space light direction

    // Sample diffuse color with parallax offset
    vec3 diffuseColor = texture(diffuseTexture, parallaxTexCoord).rgb;

    // Ambient
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diff = max(dot(tangentNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.25;
    vec3 reflectDir = reflect(-lightDir, tangentNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * diffuseColor;
    FragColor = vec4(result, 1.0f);
}
