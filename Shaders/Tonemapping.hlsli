//---------------------------------------------------------------------------
//! @file   Tonemapping.hlsli
//! @brief  HDR tone mapping operators
//---------------------------------------------------------------------------

#ifndef TONEMAPPING_HLSLI
#define TONEMAPPING_HLSLI

// ACES filmic tone mapping (Narkowicz 2015)
// 5-coefficient fit of the ACES Reference Rendering Transform.
// Maps HDR values to 0-1 with a cinematic S-curve.
// Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

#endif
