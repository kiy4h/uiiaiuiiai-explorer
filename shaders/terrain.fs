#version 450 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D terrainTexture;

// Pencahayaan
uniform vec3 lightPos;  // Posisi sumber cahaya
uniform vec3 viewPos;   // Posisi kamera
uniform vec3 lightColor; // Warna cahaya

out vec4 FragColor;

void main() {
    // Ambil warna tekstur
    vec4 texColor = texture(terrainTexture, TexCoords);

    // Normalisasi normal
    vec3 norm = normalize(Normal);

    // Arah cahaya
    vec3 lightDir = normalize(lightPos - FragPos);

    // Hitung diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);

    // Arah view/kamera
    vec3 viewDir = normalize(viewPos - FragPos);

    // Refleksi arah cahaya
    vec3 reflectDir = reflect(-lightDir, norm);

    // Hitung specular shading
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // Komponen pencahayaan
    vec3 ambient = 0.7 * lightColor; // Ambient lighting
    vec3 diffuse = diff * lightColor; // Diffuse lighting
    vec3 specular = spec * lightColor; // Specular lighting

    // Gabungkan pencahayaan dengan tekstur
    vec3 lighting = (ambient + diffuse + specular) * texColor.rgb;

    FragColor = vec4(lighting, texColor.a);
    // FragColor = texture(terrainTexture, TexCoords);
}
