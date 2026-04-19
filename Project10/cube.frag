#version 330 core
out vec4 FragColor; // Final fragment color

in vec3 FragPos;   // World-space position from vertex shader
in vec2 TexCoord;  // UV coordinates

uniform vec3 lightPos; // World-space light position
uniform vec3 viewPos;  // World-space camera position
uniform vec3 lightColor; // Light color
uniform sampler2D diffuseTexture; // Bump picture (albedo)
uniform sampler2D heightMap;      // Bump map (height source)

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTS)
{
    float heightScale = 0.08f; // Stronger parallax intensity (more obvious depth)
    float height = texture(heightMap, texCoords).r; // Height sample
    float viewZ = max(viewDirTS.z, 0.1f);           // Avoid instability at grazing angles
    vec2 p = viewDirTS.xy / viewZ * (height * heightScale); // UV offset
    return texCoords - p; // Parallax-adjusted UV
}

void main() {
    // Build tangent frame from derivatives. Used for parallax offset only.
    vec3 dp1 = dFdx(FragPos); // Position derivative x
    vec3 dp2 = dFdy(FragPos); // Position derivative y
    vec2 duv1 = dFdx(TexCoord); // UV derivative x
    vec2 duv2 = dFdy(TexCoord); // UV derivative y

    vec3 T = normalize(dp1 * duv2.y - dp2 * duv1.y); // Tangent
    vec3 B = normalize(-dp1 * duv2.x + dp2 * duv1.x); // Bitangent
    vec3 N = normalize(cross(T, B)); // Geometric normal from T/B

    vec3 geomN = normalize(cross(dp1, dp2)); // Geometric orientation reference
    if (dot(N, geomN) < 0.0) {
        N = -N; // Keep N consistent with primitive winding
        B = -B;
    }

    mat3 worldToTangent = transpose(mat3(T, B, N)); // World -> tangent transform

    vec3 tangentFragPos = worldToTangent * FragPos; // Frag position in tangent space
    vec3 tangentViewPos = worldToTangent * viewPos; // Camera position in tangent space

    vec3 viewDirTS = normalize(tangentViewPos - tangentFragPos); // Tangent-space view direction
    vec2 parallaxUV = ParallaxMapping(TexCoord, viewDirTS); // Shift UV with parallax (GL_REPEAT handles wrap)

    vec3 diffuseColor = texture(diffuseTexture, parallaxUV).rgb; // Bump picture sample

    // Cube lighting uses geometric normal only. No bump mapping here.
    vec3 lightDirWS = normalize(lightPos - FragPos); // World-space light direction
    vec3 viewDirWS = normalize(viewPos - FragPos); // World-space view direction

    float ambientStrength = 0.35; // Ambient strength
    vec3 ambient = ambientStrength * lightColor; // Ambient term

    float diff = max(dot(N, lightDirWS), 0.0); // Lambert term (flat face normal)
    vec3 diffuse = diff * lightColor; // Diffuse lighting

    float specularStrength = 0.25f; // Specular strength
    vec3 reflectDir = reflect(-lightDirWS, N); // Reflection vector
    float spec = pow(max(dot(viewDirWS, reflectDir), 0.0), 16.0); // Specular highlight
    vec3 specular = specularStrength * spec * lightColor; // Specular lighting

    vec3 result = (ambient + diffuse + specular) * diffuseColor; // Final lit color
    FragColor = vec4(result, 1.0f); // Write final color
}
