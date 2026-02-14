Texture2D SpriteSheet : register(t0);
SamplerState Sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 color = SpriteSheet.Sample(Sampler, input.uv);
    clip(color.a - 0.01);
    return color;
}