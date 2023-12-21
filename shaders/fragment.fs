#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    int shine;
};

struct Light {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cosCutoff;
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
    vec3 incidentRay = normalize(FragPos - light.position);
    vec3 lightDir = normalize(light.direction);
    vec3 eyeDir = normalize(eyePos - FragPos);
    vec3 txtrColor = vec3(texture(material.diffuse, TextureCoord));
    vec3 specColor = vec3(texture(material.specular, TextureCoord));

    //spotlight calculations
    float cosTheta = dot(incidentRay, lightDir);

    //Initialize phong model vectors
    vec3 ambient = txtrColor * light.ambient;
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if(cosTheta > light.cosCutoff) {
        //Calculate intensity of ambient color
        //...

        //Calculate intensity of diffuse colour
        diffuse = (
            (
                max(dot(norm, -incidentRay), 0.0) 
                * txtrColor
            ) 
            * light.diffuse
        );

        //Calculate intensity of specular light
        vec3 reflectionDir = incidentRay - 2 * dot(incidentRay, norm) * norm;
        specular = (
            (
                pow(max(dot(reflectionDir, eyeDir), 0.0), 
                    material.shine) 
                * specColor
            )
            * light.specular
        );
    }

    //attenuation related calculations
    float dist = length(light.position - FragPos);
    float attenuation = 1.0 /
        (light.constant + light.linear * dist
        + light.quadratic * (dist*dist));

    // Final fragment colour
    outColor = vec4(
        (ambient + diffuse + specular) * attenuation 
        * vec3(1.0),
        1.0
    );
}
