#version 330 core

#define NUM_OF_DIR_LIGHTS 1
#define NUM_OF_POINT_LIGHTS 2
#define NUM_OF_SPOT_LIGHTS 1

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 cameraPos;

float shininess = 32.0f;

#include "include/inc_helpers.glsl"
#include "include/inc_dir_light.glsl"
#include "include/inc_point_light.glsl"
#include "include/inc_spot_light.glsl"

uniform DirLight dirLight[NUM_OF_DIR_LIGHTS];
uniform PointLight pointLight[NUM_OF_POINT_LIGHTS];
uniform SpotLight spotLight[NUM_OF_SPOT_LIGHTS];

void main()
{    
    vec3 norm = normalize(normal);
    vec3 cameraDir = normalize(cameraPos - fragPos);
    vec3 result = vec3(0);

    for (int dirLightIndex = 0; dirLightIndex < NUM_OF_DIR_LIGHTS; dirLightIndex++)
    {
        result += CalcDirLight(dirLight[dirLightIndex], norm, cameraDir);
    }

    for (int pointLightIndex = 0; pointLightIndex < NUM_OF_POINT_LIGHTS; pointLightIndex++)
    {
        result += CalcPointLight(pointLight[pointLightIndex], norm, fragPos, cameraDir);
    }

    for (int spotLightIndex = 0; spotLightIndex < NUM_OF_SPOT_LIGHTS; spotLightIndex++)
    {
        result += CalcSpotLight(spotLight[spotLightIndex], norm, fragPos, cameraDir);
    }

    FragColor = vec4(result, 1.0f);
}