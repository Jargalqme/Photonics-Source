//---------------------------------------------------------------------------
//! @file   WaveWorldPS.hlsl
//! @brief  Arena floor wave effect — animated sine distortion
//---------------------------------------------------------------------------

cbuffer WaveWorldConstants : register(b0)
{
    matrix WorldViewProjection;
    float Time;
    float Speed;
    float Brightness;
    float Alpha;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // Scale world XZ to reasonable range for sine math
    float2 uv = input.uv * 0.012;

    for (float i = 1.0; i < 3.0; i++)
    {
        uv.x += 0.6 / i * cos(i * 2.5 * uv.y + Time * Speed);
        uv.y += 0.6 / i * cos(i * 1.5 * uv.x + Time * Speed);
    }

    // Clamp denominator to prevent infinity spikes (white flashes)
    float val = Brightness / max(abs(sin(Time * Speed - uv.y - uv.x)), 0.05);

    float hue = input.uv.x * 0.02 + input.uv.y * 0.02 + Time * 0.2;
    float3 col = val * float3(
          sin(hue) * 0.5 + 0.5,
          sin(hue + 2.094) * 0.5 + 0.5,
          sin(hue + 4.189) * 0.5 + 0.5
      );

    return float4(col, Alpha);
}