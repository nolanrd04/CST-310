#version 330 core
out vec4 FragColor; // Returns FragColor

in vec3 Normal; // Receives Normal
in vec3 FragPos; // Receives FragPos
in vec2 TexCoord; // Receives texture coordinate
in mat3 TBN; // Receives TBN matrix
  
uniform vec3 lightPos; // Receives lightPos uniform
uniform vec3 viewPos; // Receives viewPos uniform
uniform vec3 lightColor; // Receives lightColor uniform
uniform vec3 cylinderColor; // Receives cylinderColor uniform
uniform sampler2D normalMap; // Receives normal map sampler

void main()
{
    // Sample from normal map
    vec3 mapSample = texture(normalMap, TexCoord).rgb; // Sample texture
    
    // Convert texture to grayscale (luminance)
    float luminance = dot(mapSample, vec3(0.299, 0.587, 0.114)); // Calculate luminance
    
    // Sample normal from normal map and convert from [0,1] to [-1,1] range
    vec3 normal = normalize(mapSample * 2.0 - 1.0); // Convert from [0,1] to [-1,1]
    normal = normalize(TBN * normal); // Transform normal to world space
    
    // ambient
    float ambientStrength = 2; // Set ambient strength
    vec3 ambient = ambientStrength * lightColor; // Sets ambient
  	
    // diffuse 
    vec3 lightDir = normalize(lightPos - FragPos); // Sets light direction based on light - frag position
    float diff = max(dot(normal, lightDir), 0.0); // Diffuse contribution
    vec3 diffuse = diff * lightColor; // Sets diffuse
    
    // specular
    float specularStrength = 0.25f; // Sets specular strength
    vec3 viewDir = normalize(viewPos - FragPos); // Sets view direction
    vec3 reflectDir = reflect(-lightDir, normal); // Sets reflect direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0); // Sets specular based on power, max, and dot product
    vec3 specular = specularStrength * spec * lightColor; // Sets specular
        
    vec3 result = (ambient + diffuse + specular) * luminance; // Multiply by grayscale luminance to get colorless texture
    FragColor = vec4(result, 1.0f); // Sets vec4 based on result
} 
