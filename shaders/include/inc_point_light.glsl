//version 330 core
//file is include-only, won't work as a regular shader

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    float distance = length(fragPos - pointLight.position);
    float attenuation = GetAttenuation(pointLight.constant, pointLight.linear, pointLight.quadratic, distance);
    vec3 ambient = pointLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(pointLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = pointLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = pointLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}