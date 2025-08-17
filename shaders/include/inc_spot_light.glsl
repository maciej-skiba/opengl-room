//version 330 core
//file is include-only, won't work as a regular shader

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
    
    bool on;
};

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
    float attenuation = GetAttenuation(spotLight.constant, spotLight.linear, spotLight.quadratic, distance);

    vec3 ambient = spotLight.ambient * vec3(texture(texture_diffuse1, texCoord));
    float diffuseStrength = max(dot(normal, -lightToFragDir), 0.0f);
    vec3 diffuse = spotLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));
    vec3 reflectedLight = reflect(lightToFragDir, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = spotLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}