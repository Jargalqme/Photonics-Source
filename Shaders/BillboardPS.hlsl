//---------------------------------------------------------------------------
//! @file   BillboardPS.hlsl
//! @brief  Simple texture sample for billboard quads
//---------------------------------------------------------------------------

Texture2D tex : register(t0);
SamplerState sam : register(s0);

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return tex.Sample(sam, input.texCoord);
}