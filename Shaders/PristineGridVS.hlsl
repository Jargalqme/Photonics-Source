cbuffer GridConstants : register(b0)
{
    matrix worldViewProjection;
    float4 gridParams;
    float4 lineColor;
    float4 baseColor;
};

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 worldPos : TEXCOORD0;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0), worldViewProjection);
    output.worldPos = input.position.xz;
    return output;
}