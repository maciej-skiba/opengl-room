#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

uniform bool bloom;
uniform float bloomStrength;
uniform float exposure;
uniform float gamma;

void main()
{
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;

    if (bloom)
    {
        vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;

        hdrColor += bloomColor * bloomStrength;
    }

    // Tone mapping: HDR -> LDR
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
