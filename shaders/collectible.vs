#version 460 core

layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec2 texCoords; // Texture coordinates
layout(location = 2) in vec3 normal; // Vertex normals

out vec2 TexCoords;

uniform mat4 model;         // Model transformation matrix
uniform mat4 viewProjection; // Combined view and projection matrix

void main() {
    gl_Position = viewProjection * model * vec4(position, 1.0);
    TexCoords = texCoords; // Pass texture coordinates to the fragment shader
}
