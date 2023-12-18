#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    int shine;
};

//We're sampling from 2 textures now
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 eyePos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform Material material;

in vec3 Color;
in vec2 TextureCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 outColor;

void main() {
    //Vectors we'll reuse for various lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(FragPos - lightPos);
    vec3 eyeDir = normalize(eyePos - FragPos);

    //Calculate intensity of ambient color
    vec3 ambient = 
        material.ambient 
        * lightColor;

    //Calculate intensity of diffuse colour
    vec3 diffuse = 
        (max(dot(norm, -lightDir), 0.0) * material.diffuse) 
        * lightColor;

    //Calculate intensity of specular light
    vec3 reflectionDir = lightDir - 2 * dot(lightDir, norm) * norm;
    vec3 specular = 
        (pow(max(dot(reflectionDir, eyeDir), 0.0), material.shine) * material.specular) 
        * lightColor;

    // Color output is determined by vertex
    outColor = mix(
        texture(texture1, TextureCoord),
        texture(texture2, TextureCoord),
        0.2 // 80% of the first texture, and 20% of the second
    ) * vec4((ambient + diffuse + specular) * vec3(1.0), 1.0);
}
