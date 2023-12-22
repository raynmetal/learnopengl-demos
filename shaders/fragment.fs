#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    int shine;
};

struct Light {

    // 0 - directional
    // 1 - point
    // 2 - spot
    int type; 

    //basic light properties
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // attenuation properties
    float constant;
    float linear;
    float quadratic;

    // spotlight properties
    float cosCutoffOuter;
    float cosCutoffInner;
};
#define NR_LIGHTS 1
uniform Light lights[NR_LIGHTS];

uniform vec3 eyePos;
uniform Material material;

in vec3 Color;
in vec2 TextureCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 outColor;

vec3 calculateLight(Light light, vec3 normal, vec3 eyeDir, vec3 txtrColor, vec3 specColor);

void main() {
    vec3 norm = normalize(Normal);
    vec3 eyeDir = normalize(eyePos - FragPos);
    vec3 txtrColor = vec3(texture(material.diffuse, TextureCoord));
    vec3 specColor = vec3(texture(material.specular, TextureCoord));

    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < NR_LIGHTS; i++) {
        result += calculateLight(lights[i], norm, eyeDir, txtrColor, specColor);
    }

    outColor = vec4(result, 1.0);
}

vec3 calculateLight(Light light, vec3 norm, vec3 eyeDir, vec3 txtrColor, vec3 specColor) {
    //Vectors we'll reuse for various lighting calculations
    vec3 incidentRay = normalize(FragPos - light.position);
    vec3 lightDir = normalize(light.direction);
    if(light.type == 0) { // directional lights are defined in terms of their direction alone
        incidentRay = lightDir;
    }

    //Calculate intensity of ambient color
    vec3 ambient = txtrColor * light.ambient;

    //Calculate intensity of diffuse colour
    vec3 diffuse = (
        (
            max(dot(norm, -incidentRay), 0.0) 
            * txtrColor
        ) 
        * light.diffuse
    );

    //Calculate intensity of specular light
    vec3 reflectionDir = incidentRay - 2 * dot(incidentRay, norm) * norm;
    vec3 specular = (
        (
            pow(max(dot(reflectionDir, eyeDir), 0.0), 
                material.shine) 
            * specColor
        )
        * light.specular
    );

    //attenuation related calculations
    float attenuation = 1.0;
    if(light.type == 1 || light.type == 2){ // point or spot lights
        float dist = length(light.position - FragPos);
        attenuation = 1.0 /
            (light.constant + light.linear * dist
            + light.quadratic * (dist*dist));
    }

    //spotlight calculations
    float spotIntensity = 1.0;
    if(light.type == 2){ // spot lights only
        float cosTheta = dot(incidentRay, lightDir);
        spotIntensity = clamp(
            (cosTheta - light.cosCutoffOuter) / (light.cosCutoffInner - light.cosCutoffOuter),
            0.0, 1.0
        );
    }

    // fragment colour contribution from this lightsource
    return (ambient + (diffuse + specular) * spotIntensity) * attenuation;
}
