cbuffer MenuConstants : register(b0)
{
    float Time;
    float2 Resolution;
    float Speed;
    float PatternScale;
    float WarpIntensity;
    float Brightness;
    float ChromaticOffset;
    float3 ColorTint;
    float Padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 fragCoord = input.uv * Resolution;
    float2 uv = (2.0 * fragCoord - Resolution) / min(Resolution.x, Resolution.y);

    for (float i = 1.0; i < 5.0; i++)
    {
        uv.x += 0.6 / i * cos(i * 2.5 * uv.y + Time);
        uv.y += 0.6 / i * cos(i * 1.5 * uv.x + Time);
    }

    float val = 0.1 / abs(sin(Time - uv.y - uv.x));
    float3 col = float3(val, val, val);
    col *= ColorTint;

    return float4(col, 1.0);
}