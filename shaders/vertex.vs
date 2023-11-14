#version 330 core

// The vertex shader's duty is to take as input a vertex position,
// and place it as output the final vertex position in 
// normalized device coordinates

layout (location = 0) in vec2 position;
in vec3 color;
in vec2 textureCoord;

out vec3 Color;
out vec2 TextureCoord;

void main() {
    // Vertex position  is same as input
    gl_Position = vec4(position, 0.0, 1.0);
    Color = color;
    TextureCoord = textureCoord;
}
