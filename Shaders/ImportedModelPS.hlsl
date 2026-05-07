//---------------------------------------------------------------------------
//! @file   ImportedModelPS.hlsl
//! @brief  Material-color and base-color texture pass for imported models.
//---------------------------------------------------------------------------

Texture2D BaseColorTexture : register(t0);
SamplerState BaseColorSampler : register(s0);

cbuffer ImportedModelCB : register(b0)
{
    float4x4 WorldViewProjection;
    float4x4 WorldInverseTranspose;
    float4 TintColor;
    float4 LightDirectionAndAmbient;
    float4 MaterialFlags;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(-LightDirectionAndAmbient.xyz);
    float ambient = LightDirectionAndAmbient.w;
    float diffuse = saturate(dot(normal, lightDirection));
    float lighting = saturate(ambient + diffuse * 0.85);

    float4 baseColor = TintColor;
    if (MaterialFlags.x > 0.5)
    {
        baseColor *= BaseColorTexture.Sample(BaseColorSampler, input.texCoord);
    }

    return float4(baseColor.rgb * lighting, baseColor.a);
}
