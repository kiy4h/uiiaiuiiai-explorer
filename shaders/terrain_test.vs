#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(position, 1.0)); // World position of the vertex
    Normal = mat3(transpose(inverse(model))) * normal; // Correct normals for transformations
    TexCoords = texCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0); // Transform to clip space
}
