#version 330 core

// The vertex shader's duty is to take as input a vertex position,
// and place it as output the final vertex position in 
// normalized device coordinates

in vec3 position;
in vec3 color;
in vec2 textureCoord;
in vec3 normal;

// Model-View-Projection matrices; see https://jsantell.com/model-view-projection/
uniform mat4 model;
uniform mat4 normalMat;
uniform mat4 view;
uniform mat4 projection;

out vec3 Color;
out vec2 TextureCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    // Vertex position is transformed by our MVP matrices
    gl_Position = projection * view * model * vec4(position, 1.0);
    Color = color;
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = vec3(normalMat * vec4(normal, 0.0));
    TextureCoord = textureCoord;
}
