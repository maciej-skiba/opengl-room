//version 330 core
//file is include-only, won't work as a regular shader

struct DirLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir)
{
    vec3 ambient = dirLight.ambient * vec3(texture(texture_diffuse1, texCoord));

    vec3 lightDirection = normalize(dirLight.position - fragPos);
    float diffuseStrength = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = dirLight.diffuse * diffuseStrength * vec3(texture(texture_diffuse1, texCoord));

    vec3 reflectedLight = reflect(-lightDirection, normal);
    float spec = pow(max(dot(reflectedLight, viewDir), 0.0f), shininess);
    vec3 specular = dirLight.specular * vec3(texture(texture_specular1, texCoord)) * spec;

    return ambient + diffuse + specular;
}