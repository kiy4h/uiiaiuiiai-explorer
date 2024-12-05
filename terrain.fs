#version 330 core

out vec4 FragColor;

in float vertexHeight; // Receive height from vertex shader
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    // Normalize the height to [0, 1] (adjust max height based on your terrain)
    float normalizedHeight = clamp(vertexHeight / 50.0, 0.0, 1.0); // Example: max height = 50

    // Use a gradient to define color based on height
    vec3 deepColor = vec3(0.1, 0.2, 0.4);   // Dark blue for lower areas
    vec3 shallowColor = vec3(0.6, 0.9, 0.3); // Green for higher areas
    vec3 terrainColor = mix(deepColor, shallowColor, normalizedHeight);

    FragColor = vec4(terrainColor, 1.0); // Final color
    // FragColor = texture(texture_diffuse1, TexCoords);
}