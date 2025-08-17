#version 330 core

#define NUM_OF_DIR_LIGHTS 1
#define NUM_OF_POINT_LIGHTS 2
#define NUM_OF_SPOT_LIGHTS 1
#define NUM_OF_ATTENUATIONS 12

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 position;
    float lightStrength;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float lightStrength;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float lightStrength;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
    
    bool on;
};

struct Attenuation {
    float minDist;
    float maxDist;
    float constant;
    float linear;
    float quadratic;
};

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 cameraPos;

uniform DirLight dirLight[NUM_OF_DIR_LIGHTS];
uniform PointLight pointLight[NUM_OF_POINT_LIGHTS];
uniform SpotLight spotLight[NUM_OF_SPOT_LIGHTS];
uniform Attenuation attenuations[NUM_OF_ATTENUATIONS];

float shininess = 48.0f;

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight spotLight, vec3 normal, vec3 fragPos, vec3 viewDir);
float GetAttenuation(float distance);

void main()
{    
    vec3 norm = normalize(normal);
    vec3 cameraDir = normalize(cameraPos - fragPos);
    vec3 result = vec3(0);

    for (int dirLightIndex = 0; dirLightIndex < NUM_OF_DIR_LIGHTS; dirLightIndex++)
    {
        //result += CalcDirLight(dirLight[dirLightIndex], norm, cameraDir);
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

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir)
{
    vec3 ambient = dirLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(dirLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f) * dirLight.lightStrength;
    vec3 diffuse = dirLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    float roughness = texture(texture_specular1, texCoord).r;
    float glossiness = 1.0f - roughness;
    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = dirLight.specular * glossiness * spec;

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float distance = length(fragPos - pointLight.position);
    float attenuation = GetAttenuation(distance);
    vec3 ambient = pointLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(pointLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f) * pointLight.lightStrength;
    vec3 diffuse = pointLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    float roughness = texture(texture_specular1, texCoord).r;
    float glossiness = 1.0f - roughness;
    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = pointLight.specular * glossiness * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= sqrt(attenuation);

    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!spotLight.on) return vec3(0.0f);
    
    vec3 lightToFragDir = normalize(fragPos - spotLight.position);
    float thetaDotProduct = dot(lightToFragDir, normalize(spotLight.direction));

    if (thetaDotProduct < spotLight.outerCutOff)
        return vec3(0.0f);

    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((thetaDotProduct - spotLight.outerCutOff) / epsilon, 0.0, 1.0);

    float distance = length(fragPos - spotLight.position);
    float attenuation = GetAttenuation(distance);

    vec3 ambient = spotLight.ambient * vec3(texture(texture_diffuse1, texCoord));
    float diffuseStrength = max(dot(normal, -lightToFragDir), 0.0f) * spotLight.lightStrength;
    vec3 diffuse = spotLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    float roughness = texture(texture_specular1, texCoord).r;
    float glossiness = 1.0f - roughness;
    vec3 reflectedLight = reflect(-lightToFragDir, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = spotLight.specular * glossiness * spec;

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}

float GetAttenuation(float distance)
{
    Attenuation attForDistance;
    for (int attIndex = 0; attIndex < NUM_OF_ATTENUATIONS; attIndex++) 
    {
        if (distance >= attenuations[attIndex].minDist && distance < attenuations[attIndex].maxDist) 
        {
            attForDistance = attenuations[attIndex];
            break;
        }
        else 
        {
            if (attIndex == NUM_OF_ATTENUATIONS - 1)
            {
                attForDistance = attenuations[NUM_OF_ATTENUATIONS - 1];
            }
        }
    }

    return 1.0 / (attForDistance.constant + attForDistance.linear * distance + attForDistance.quadratic * (distance * distance));
}