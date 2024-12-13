#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal; // Tambahkan normal vector sebagai input

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 FragPos;  // Untuk posisi fragmen dalam world space
out vec3 Normal;   // Untuk normal fragmen dalam world space

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    TexCoords = texCoords;

    // Hitung posisi fragmen dalam world space
    FragPos = vec3(model * vec4(position, 1.0));

    // Hitung normal dalam world space
    Normal = mat3(transpose(inverse(model))) * normal; // Transformasi normal
}
