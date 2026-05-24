//---------------------------------------------------------------------------
//! @file   PS_SkinnedMenu.hlsl
//! @brief  Pixel shader for the menu skinned-character path. Mirrors
//!         PS_ImportedModel.hlsl — simple Lambert with optional base-color
//!         texture and tint.
//---------------------------------------------------------------------------

Texture2D    BaseColorTexture : register(t0);
SamplerState BaseColorSampler : register(s0);

cbuffer SkinnedTransformCB : register(b0)
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

    float emissiveIntensity = MaterialFlags.y;
    return float4(baseColor.rgb * lighting * (1.0 + emissiveIntensity), baseColor.a);
}
