#include <vector>

struct Attenuation {
    float minDist;
    float maxDist;
    float constant;
    float linear;
    float quadratic;
};

std::vector<Attenuation> attenuationData = {
    {0.0f,    13.0f,   1.0f, 0.35f,   0.9f},
    {13.0f,   20.0f,   1.0f, 0.18f,   0.22f},
    {20.0f,   32.0f,   1.0f, 0.11f,   0.10f},
    {32.0f,   50.0f,   1.0f, 0.07f,   0.035f},
    {50.0f,   65.0f,   1.0f, 0.045f,  0.016f},
    {65.0f,   100.0f,  1.0f, 0.035f,  0.0085f},
    {100.0f,  160.0f,  1.0f, 0.022f,  0.0037f},
    {160.0f,  200.0f,  1.0f, 0.014f,  0.0014f},
    {200.0f,  325.0f,  1.0f, 0.011f,  0.0009f},
    {325.0f,  600.0f,  1.0f, 0.007f,  0.00035f},
    {600.0f,  3250.0f,1.0f, 0.0035f,  0.0001f},
    {3250.0f, 99999.0f,1.0f,0.0007f,  0.0000035f}
};