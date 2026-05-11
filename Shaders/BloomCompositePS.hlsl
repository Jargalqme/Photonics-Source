//---------------------------------------------------------------------------
//! @file   BloomCompositePS.hlsl
//! @brief  Final bloom composite — scene + bloom with intensity/exposure
//---------------------------------------------------------------------------

#include "Common.hlsli"
#include "Tonemapping.hlsli"

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

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 scene = sceneTexture.Sample(linearSampler, input.uv).rgb;
    float3 bloom = bloomTexture.Sample(linearSampler, input.uv).rgb;

    float3 color = scene + bloom * bloomIntensity;
    color *= exposure;
    color = ACESFilm(color);

    return float4(color, 1.0);
}