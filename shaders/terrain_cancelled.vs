#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

out vec2 TexCoords;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);

    // Pass texture coordinates
    TexCoords = texCoords;

    // Pass fragment position in world space
    FragPos = vec3(model * vec4(position, 1.0));

    // Calculate Tangent-Bitangent-Normal matrix (TBN)
    vec3 T = normalize(mat3(model) * tangent);
    vec3 N = normalize(mat3(model) * normal);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
}
