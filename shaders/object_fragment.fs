#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_diffuse4;

    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_specular3;
    sampler2D texture_specular4;
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

#define NR_LIGHTS 6
uniform Light lights[NR_LIGHTS];

uniform vec3 eyePos;
uniform Material material;
uniform float nearDepth;
uniform float farDepth;

in vec3 Color;
in vec2 TextureCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 outColor;

/*
Returns the fragment colour contribution from a particular light source, given
a material's texture and specular colour at this point
*/
vec3 calculateLight(Light light, vec3 normal, vec3 eyeDir, vec3 txtrColor, vec3 specColor);

void main() {
    vec3 norm = normalize(Normal);
    vec3 eyeDir = normalize(eyePos - FragPos);
    vec4 txtrColor = texture(material.texture_diffuse1, TextureCoord);
    if(txtrColor.a < 0.1) discard;
    vec3 specColor = vec3(texture(material.texture_specular1, TextureCoord));

    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < NR_LIGHTS; i++) {
        result += calculateLight(lights[i], norm, eyeDir, txtrColor.rgb, specColor);
    }

    outColor = vec4(result, txtrColor.a);
    //Convert depth value to pre-NDC equivalent
    // float ndc = gl_FragCoord.z*2.0 - 1.0;
    // float linearDepth = (2.0*nearDepth*farDepth)/(farDepth+nearDepth - ndc*(farDepth - nearDepth));
    // outColor = vec4(vec3(linearDepth/farDepth), 1.0);
}

vec3 calculateLight(Light light, vec3 norm, vec3 eyeDir, vec3 txtrColor, vec3 specColor) {
    //Vectors we'll reuse for various lighting calculations
    vec3 incidentRay = normalize(FragPos - light.position);
    vec3 lightDir = normalize(light.direction);

    // Directional lights are defined in terms of their direction alone
    if(light.type == 0) { 
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
                32) 
            * specColor
        )
        * light.specular
    );

    // Attenuation related calculations
    float attenuation = 1.0;
    if(light.type == 1 || light.type == 2){ // point or spot lights
        float dist = length(light.position - FragPos);
        attenuation = 1.0 /
            (light.constant + light.linear * dist
            + light.quadratic * (dist*dist));
    }

    // Spotlight calculations
    float spotIntensity = 1.0;
    if(light.type == 2){ // spot lights only
        float cosTheta = dot(incidentRay, lightDir);
        spotIntensity = clamp(
            (cosTheta - light.cosCutoffOuter) / (light.cosCutoffInner - light.cosCutoffOuter),
            0.0, 1.0
        );
    }

    // Fragment colour contribution from this lightsource
    return (ambient + (diffuse + specular) * spotIntensity) * attenuation;
}
