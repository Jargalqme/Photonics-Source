//---------------------------------------------------------------------------
//! @file   PS_ImportedModel.hlsl
//! @brief  Physically based (Cook-Torrance) shading for imported models.
//---------------------------------------------------------------------------

#include "BRDF.hlsli"

Texture2D    BaseColorTexture  : register(t0);
Texture2D    NormalTexture     : register(t1);
Texture2D    MetalRoughTexture : register(t2);
TextureCube  IrradianceCube    : register(t3);
SamplerState BaseColorSampler  : register(s0);

cbuffer ImportedModelCB : register(b0)
{
    float4x4 World;
    float4x4 WorldViewProjection;
    float4x4 WorldInverseTranspose;
    float4   TintColor;
    float4   LightDirectionAndAmbient;
    float4   LightColor;
    float4   CameraPosition;
    float4   MaterialParams;
    float4   MaterialFlags;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent  : TANGENT;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // --- world-space normal (with normal map) ---
    float3 N = normalize(input.normal);
    if (MaterialParams.z > 0.5)   // hasNormalMap
    {
        float3 T = normalize(input.tangent - N * dot(N, input.tangent)); // Gram-Schmidt
        float3 B = cross(N, T);
        float3 tn = NormalTexture.Sample(BaseColorSampler, input.texCoord).xyz * 2.0 - 1.0;
        N = normalize(mul(tn, float3x3(T, B, N))); // tangent -> world
    }

    // --- material inputs ---
    float4 base = TintColor;
    if (MaterialFlags.x > 0.5)
        base *= BaseColorTexture.Sample(BaseColorSampler, input.texCoord); // sRGB -> linear
    float3 albedo = base.rgb;

    float metallic = MaterialParams.x;
    float roughness = MaterialParams.y;
    if (MaterialParams.w > 0.5)   // hasMRMap
    {
        float2 mr = MetalRoughTexture.Sample(BaseColorSampler, input.texCoord).gb; // G=rough, B=metal
        roughness *= mr.x;
        metallic *= mr.y;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    // --- direct lighting (Cook-Torrance) ---
    float3 V = normalize(CameraPosition.xyz - input.worldPos);
    float3 L = normalize(-LightDirectionAndAmbient.xyz); // toward the light
    float NoL = saturate(dot(N, L));

    float3 F0 = lerp((float3) 0.04, albedo, metallic); // dielectric ~4%, metal = albedo
    float3 specular = CookTorranceSpecular(N, V, L, roughness, F0);

    float3 H = normalize(V + L);
    float3 F = F_Schlick(max(dot(H, V), 0.0), F0);
    float3 kD = (1.0 - F) * (1.0 - metallic); // metals have no diffuse
    float3 diffuse = kD * albedo / PI;

    float3 Lo = (diffuse + specular) * LightColor.rgb * NoL;

    // --- placeholder ambient until IBL (Stage 2) ---
    float3 irradiance = IrradianceCube.Sample(BaseColorSampler, N).rgb;
    float3 ambient = irradiance * albedo * (1.0 - metallic);
    float emissive = MaterialFlags.y;
    float3 color = Lo + ambient + albedo * emissive;

    return float4(color, base.a); // linear HDR; ACES tonemapper finishes it
}
