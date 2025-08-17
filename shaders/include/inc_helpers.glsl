//version 330 core
//file is include-only, won't work as a regular shader

float GetAttenuation(float constant, float linear, float quadratic, float distance)
{
    return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}