//---------------------------------------------------------------------------
//! @file   SceneCopyPS.hlsl
//! @brief  Pass-through pixel shader. Samples the input texture and writes
//          the color to the bound RTV. Used as the final composite step
//          from HDR scene (or bloom output) into the BGRA backbuffer.
//---------------------------------------------------------------------------

Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return sceneTexture.Sample(sceneSampler, input.uv);
}