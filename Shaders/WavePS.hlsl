//---------------------------------------------------------------------------
//! @file   WavePS.hlsl
//! @brief  Iterative sine wave distortion menu background effect
//---------------------------------------------------------------------------

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

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 fragCoord = input.uv * Resolution;
    float2 uv = (2.0 * fragCoord - Resolution) / min(Resolution.x, Resolution.y);

    for (float i = 1.0; i < 8.0; i++)
    {
        uv.x += 0.6 / i * cos(i * 2.5 * uv.y + Time);
        uv.y += 0.6 / i * cos(i * 1.5 * uv.x + Time);
    }

    float val = 0.1 / abs(sin(Time - uv.y - uv.x));
    //float3 col = float3(val, val, val);
    //col *= ColorTint;
    
    // Hue shift based on wave intensity and time
  //  float hue = uv.x * 0.3 + uv.y * 0.2 + Time * 0.1;
  //  float3 col = val * float3(
  //    sin(hue) * 0.3 + 0.7,
  //    sin(hue + 2.094) * 0.3 + 0.7, // 2.094 = 2*PI/3
  //    sin(hue + 4.189) * 0.3 + 0.7 // 4.189 = 4*PI/3
  //);
  //  col *= ColorTint;
    
    
    float3 color1 = float3(0.0, 0.4, 1.0); // electric blue
    float3 color2 = float3(0.6, 0.0, 1.0); // purple
    float blend = sin(uv.y * 2.0 + Time * 0.3) * 0.5 + 0.5;
    float3 col = val * lerp(color1, color2, blend);
    
    //float pulse = sin(Time * 0.5) * 0.15 + 1.0; // 0.85 to 1.15
    //float3 col = float3(val * pulse, val * pulse, val * pulse);
    //col *= ColorTint;
    
  //  float hue = uv.x * 0.8 + uv.y * 0.5 + Time * 0.8;
  //  float3 col = val * float3(
  //    sin(hue) * 0.3 + 0.7,
  //    sin(hue + 2.094) * 0.5 + 0.5,
  //    sin(hue + 4.189) * 0.5 + 0.5
  //);
    
    return float4(col, 1.0);
}