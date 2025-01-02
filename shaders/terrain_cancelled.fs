#version 460 core

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D baseColorMap;  // Base color
uniform sampler2D normalMap;     // Normal map
uniform sampler2D heightMap;     // Height map
uniform sampler2D roughnessMap;  // Roughness map
uniform sampler2D aoMap;         // Ambient occlusion map

uniform vec3 viewPos;

out vec4 FragColor;

// POM parameters
const int numLayers = 32;          // Number of layers for heightmap sampling
const float heightScale = 0.1;     // Scale of heightmap (adjust to fit your terrain)

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir) {
    float layerDepth = 1.0 / float(numLayers);
    float currentLayerDepth = 0.0;

    vec2 P = viewDir.xy * heightScale; // Parallax vector
    vec2 deltaTexCoords = P / float(numLayers);

    vec2 currentTexCoords = texCoords;

    // Perform parallax occlusion mapping iterations
    for (int i = 0; i < numLayers; i++) {
        float height = texture(heightMap, currentTexCoords).r; // Sample heightmap
        if (currentLayerDepth > height) {
            break;
        }
        currentTexCoords -= deltaTexCoords;
        currentLayerDepth += layerDepth;
    }

    // Perform linear interpolation between the last two points
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterHeight = texture(heightMap, currentTexCoords).r;
    float beforeHeight = texture(heightMap, prevTexCoords).r;

    float weight = (currentLayerDepth - afterHeight) / (beforeHeight - afterHeight);
    return mix(currentTexCoords, prevTexCoords, weight);
}

void main() {
    // Calculate view direction in tangent space
    vec3 viewDir = normalize(TBN * (viewPos - FragPos));

    // Perform parallax mapping
    vec2 parallaxTexCoords = parallaxMapping(TexCoords, viewDir);

    // Sample texture maps using parallax-corrected coordinates
    vec3 baseColor = texture(baseColorMap, parallaxTexCoords).rgb;
    vec3 normal = texture(normalMap, parallaxTexCoords).rgb * 2.0 - 1.0;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    // Lighting calculations (example, you can customize)
    vec3 ambient = ao * baseColor * 0.1;
    vec3 diffuse = baseColor; // Add proper lighting here if needed

    FragColor = vec4(ambient + diffuse, 1.0);
}
