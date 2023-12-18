#version 330 core

struct Material {
    sampler2D diffuse;
    vec3 specular;
    int shine;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 eyePos;
uniform Light light;
uniform Material material;

in vec3 Color;
in vec2 TextureCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 outColor;

void main() {
    //Vectors we'll reuse for various lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(FragPos - light.position);
    vec3 eyeDir = normalize(eyePos - FragPos);
    vec3 txtrColor = vec3(texture(material.diffuse, TextureCoord));

    //Calculate intensity of ambient color
    vec3 ambient = (
        txtrColor
        * light.ambient
    );

    //Calculate intensity of diffuse colour
    vec3 diffuse = (
        (
            max(dot(norm, -lightDir), 0.0) 
            * txtrColor
        ) 
        * light.diffuse
    );

    //Calculate intensity of specular light
    vec3 reflectionDir = lightDir - 2 * dot(lightDir, norm) * norm;
    vec3 specular = (
        (
            pow(max(dot(reflectionDir, eyeDir), 0.0), 
                material.shine) 
            * material.specular
        )
        * light.specular
    );

    // Color output is determined by vertex
    outColor = vec4((ambient + diffuse + specular) * vec3(1.0), 1.0);
}
