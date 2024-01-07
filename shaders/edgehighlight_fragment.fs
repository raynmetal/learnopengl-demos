#version 330 core

const float offset = 1.0/300.0;

uniform sampler2D screenTexture;

in vec2 TextureCoord;

out vec4 FragColor;

void main() {
    vec2 offsets[9] = vec2[](
        //top
        vec2(-offset, offset), //top-left
        vec2(0.f, offset), //top-mid
        vec2(offset, offset), //top-right
        //mid
        vec2(-offset, 0.f), //mid-left
        vec2(0.f, 0.f), //mid-mid
        vec2(offset, 0.f), //mid-right
        //bottom
        vec2(-offset, -offset), //bottom-left
        vec2(0.f, -offset), //bottom-mid
        vec2(offset, -offset) //bottom-right
    );

    // Weights applied to each of the samples in
    // the area around our current texel
    float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

    // texture samples around and including current 
    // texel
    vec3 sampleTex[9];
    for(int i = 0; i < 9; ++i){
        sampleTex[i] = vec3(texture(screenTexture, TextureCoord.st + offsets[i]));
    }

    // Weighted addition of texture samples
    vec3 aggregateColor = vec3(0.f);
    for(int i = 0; i < 9; ++i){
        aggregateColor += sampleTex[i] * kernel[i];
    }

    FragColor = vec4(aggregateColor, 1.f);
}
