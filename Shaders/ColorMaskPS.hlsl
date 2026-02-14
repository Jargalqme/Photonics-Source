cbuffer ColorMaskCB : register(b0)
{
    float3 colorMask;
    float padding;
};

Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 color = sceneTexture.Sample(sceneSampler, input.uv);
    color.rgb *= colorMask;
    return color;
}

