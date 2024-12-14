#version 460 core

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

#define MAX_LIGHTS 10
uniform PointLight pointLights[MAX_LIGHTS];
uniform int numPointLights;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

out vec4 FragColor;

vec3 calculateBlinnPhong(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // Light direction
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse shading (Lambertian reflection model)
    float diff = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong Specular Shading
    vec3 halfDir = normalize(lightDir + viewDir); // Halfway vector
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0); // Blinn-Phong specular

    // Combine results
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 specular = spec * light.color * 0.5; // Specular intensity

    return diffuse + specular;
}

void main() {
    vec3 norm = normalize(Normal); // Normalize the normal
    vec3 viewDir = normalize(viewPos - FragPos); // View direction

    // Base color from texture
    vec4 texColor = texture(diffuseTexture, TexCoords);

    // Ambient light contribution
    vec3 ambient = 0.5 * vec3(1.0, 1.0, 1.0); // Soft white ambient light
    vec3 result = ambient; // Start with ambient light

    // Accumulate light contribution from point lights
    // for (int i = 0; i < numPointLights; ++i) {
    //     result += calculateBlinnPhong(pointLights[i], norm, FragPos, viewDir);
    // }

    // Final color: base texture color modulated by light contributions
    FragColor = vec4(texColor.rgb * result, texColor.a);
}
