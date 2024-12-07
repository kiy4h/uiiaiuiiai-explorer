#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Texture coordinates

out vec3 FragPos; // Pass position to fragment shader
out vec3 Normal;  // Pass normal to fragment shader
out vec2 TexCoords; // Pass texture coordinates

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Transform position by model matrix
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal by model matrix
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0); // Project to screen space
}