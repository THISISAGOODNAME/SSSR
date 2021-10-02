#pragma once

#define PI 3.14159265358979323846f

float LinearToSrgb(float channel)
{
    if (channel <= 0.0031308f)
    {
        return 12.92f * channel;
    }
    else
    {
        return 1.055f * pow(channel, 1.0f / 2.4f) - 0.055f;
    }
}

float3 LinearToSrgb(float3 linear)
{
    return float3(LinearToSrgb(linear.r), LinearToSrgb(linear.g), LinearToSrgb(linear.b));
}

float LinearToGamma(float channel)
{
    float exponent = 1.0f / 2.2f;
    return pow(channel, exponent);
}

float3 LinearToGamma(float3 channel)
{
    float exponent = 1.0f / 2.2f;
    return pow(channel, exponent);
}
