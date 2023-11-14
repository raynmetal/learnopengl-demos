#version 330 core

uniform float uniColor;
uniform float texture2CoordMultiplier;

//We're sampling from 2 textures now
uniform sampler2D texture1;
uniform sampler2D texture2;

in vec3 Color;
in vec2 TextureCoord;

out vec4 outColor;

void main() {

    // Color output is determined by vertex
    outColor = mix(
        texture(texture1, TextureCoord),
        texture(texture2, TextureCoord * texture2CoordMultiplier),
        0.2 // 80% of the first texture, and 20% of the second
    );
}
