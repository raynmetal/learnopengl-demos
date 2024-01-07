#version 330 core

uniform sampler2D screenTexture;

in vec2 TextureCoord;

out vec4 FragColor;

void main() {
    FragColor = vec4(vec3(1.0 - texture(screenTexture, TextureCoord)), 1.0);
}
