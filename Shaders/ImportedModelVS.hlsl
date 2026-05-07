//---------------------------------------------------------------------------
//! @file   ImportedModelVS.hlsl
//! @brief  Vertex shader for imported Assimp model buffers.
//---------------------------------------------------------------------------

cbuffer ImportedModelCB : register(b0)
{
    float4x4 WorldViewProjection;
    float4x4 WorldInverseTranspose;
    float4 TintColor;
    float4 LightDirectionAndAmbient;
    float4 MaterialFlags;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = mul(float4(input.position, 1.0), WorldViewProjection);
    output.normal = normalize(mul(float4(input.normal, 0.0), WorldInverseTranspose).xyz);
    output.texCoord = input.texCoord;

    return output;
}
