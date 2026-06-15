#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 ourColor;

uniform float emissionStrength;

void main()
{
    // Kolor normalnie widoczny w scenie
    FragColor = vec4(ourColor, 1.0);

    // Kolor, który pójdzie do tekstury bloom/bright
    BrightColor = vec4(ourColor * emissionStrength, 1.0);
}