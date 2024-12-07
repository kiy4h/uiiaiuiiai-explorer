
#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;  
uniform vec3 viewPos;   
uniform vec3 objectColor;  
uniform vec3 lightColor;  
uniform float shininess;

uniform sampler2D texture_diffuse1; // For terrain texture

void main()
{
    // Ambient
    float ambientStrength = 0.7f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    vec3 specular = spec * lightColor;

    // Combine results
    vec3 result = (ambient + diffuse + specular) * objectColor;

    // Apply texture
    FragColor = texture(texture_diffuse1, TexCoords) * vec4(result, 1.0f);
}