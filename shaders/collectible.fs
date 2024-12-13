#version 460 core

in vec2 TexCoords;

uniform sampler2D texture_diffuse; // The main texture of the collectible
uniform vec3 emissiveColor;        // The emissive light color
uniform float emissiveIntensity;   // Intensity of the emissive effect

out vec4 FragColor;

void main() {
    vec4 baseColor = texture(texture_diffuse, TexCoords); // Sample texture color
    vec3 emissive = emissiveColor * emissiveIntensity;    // Add emissive lighting
    FragColor = vec4(baseColor.rgb + emissive, baseColor.a); // Combine texture and emissive lighting
}
