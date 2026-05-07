//---------------------------------------------------------------------------
//! @file   SkyboxVS.hlsl
//! @brief  Fullscreen triangle at depth=1 with world direction from inverse VP
//---------------------------------------------------------------------------

cbuffer SkyboxConstants : register(b0)
{
    matrix inverseViewProjection;
};

struct VS_OUTPUT
{
    float4 position  : SV_POSITION;
    float3 direction : TEXCOORD0;
};

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;

    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, 1.0, 1.0);

    float4 world = mul(output.position, inverseViewProjection);
    output.direction = world.xyz / world.w;

    return output;
}
