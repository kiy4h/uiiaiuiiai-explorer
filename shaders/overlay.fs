#version 460 core
out vec4 FragColor;

uniform vec4 overlayColor;

void main() {
    FragColor = overlayColor;
}
