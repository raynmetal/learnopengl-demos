#version 330 core

// The vertex shader's duty is to take as input a vertex position,
// and place it as output the final vertex position in 
// normalized device coordinates

in vec2 position;
in vec3 color;
in vec2 textureCoord;

// Model-View-Projection matrices; see https://jsantell.com/model-view-projection/
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
out vec2 TextureCoord;

void main() {
    // Vertex position is transformed by our MVP matrices
    gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
    Color = color;
    TextureCoord = textureCoord;
}
