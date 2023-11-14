#version 330 core

uniform float uniColor;
uniform float texture2CoordMultiplier;

//We're sampling from 2 textures now
uniform sampler2D texture1;
uniform sampler2D texture2;

in vec3 Color;
in vec2 TextureCoord;
in vec2 Position;

out vec4 outColor;

void main() {
    float manhattanDistance = 
        (Position.x>0?Position.x:-Position.x)
        + (Position.y>0?Position.y:-Position.y);
    if(manhattanDistance > 1.0) manhattanDistance = 1.0;

    // Color output is determined by vertex
    outColor = vec4(
        mix(
            texture(texture1, TextureCoord),
            texture(texture2, TextureCoord * texture2CoordMultiplier),
            0.2 // 80% of the first texture, and 20% of the second
        ).xyz, 
        1.0 - manhattanDistance
    );
}
