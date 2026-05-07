//---------------------------------------------------------------------------
//! @file   BloomCompositePS.hlsl
//! @brief  Final bloom composite — scene + bloom with intensity/exposure
//---------------------------------------------------------------------------

#include "Common.hlsli"

cbuffer BloomParams : register(b0)
{
    float2 texelSize;
    float  sampleScale;
    float  padding1;
    float4 threshold;
    float  bloomIntensity;
    float  exposure;
    float2 padding2;
};

Texture2D    sceneTexture : register(t0);
Texture2D    bloomTexture : register(t1);
SamplerState linearSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

// ACES filmic tone mapping (Narkowicz 2015)
// Maps HDR values to 0-1 with a cinematic S-curve
float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 scene = sceneTexture.Sample(linearSampler, input.uv).rgb;
    float3 bloom = bloomTexture.Sample(linearSampler, input.uv).rgb;

    float3 color = scene + bloom * bloomIntensity;
    color *= exposure;
    color = ACESFilm(color);

    return float4(color, 1.0);
}