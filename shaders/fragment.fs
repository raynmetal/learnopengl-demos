#version 330 core

//We're sampling from 2 textures now
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float ambientStrength;

in vec3 Color;
in vec2 TextureCoord;

out vec4 outColor;

void main() {
    vec3 ambientLightColor = ambientStrength * lightColor;
    // Color output is determined by vertex
    outColor = mix(
        texture(texture1, TextureCoord),
        texture(texture2, TextureCoord),
        0.2 // 80% of the first texture, and 20% of the second
    ) * vec4(ambientLightColor * objectColor, 1.0);
}
