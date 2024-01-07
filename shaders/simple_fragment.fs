#version 330 core

uniform sampler2D screenTexture;

in vec2 TextureCoord;

out vec4 FragColor;

void main() {
    FragColor = texture(screenTexture, TextureCoord);
    float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    FragColor = vec4(vec3(average), 1.0);
}
