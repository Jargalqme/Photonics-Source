cbuffer MenuConstants : register(b0)
{
    float  Time;
    float2 Resolution;
    float  Speed;
    float  PatternScale;
    float  WarpIntensity;
    float  Brightness;
    float  ChromaticOffset;
    float3 ColorTint;
    float  Padding;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.uv = input.uv;
    return output;
}