cbuffer BeamConstants : register(b0)
{
    matrix ViewProjection;
    float3 BeamStart;
    float BeamWidth;
    float3 BeamEnd;
    float BeamLife;
    float4 BeamColor;
    float3 CameraPosition;
    float Time;
};

struct VSInput
{
    float3 position : POSITION; // Vertex position in object space.
    float2 uv : TEXCOORD0;      // Texture coordinates
};

struct PSInput
{
    float4 position : SV_POSITION; // SV_ System-Value Semantics
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

PSInput main(VSInput input)
{
    PSInput output;
    
    
    float3 beamPos = lerp(BeamStart, BeamEnd, input.uv.x);
    float3 beamDir = normalize(BeamEnd - BeamStart);
    float3 toCamera = normalize(CameraPosition - beamPos);
    float3 right = normalize(cross(beamDir, toCamera));
    float offset = (input.uv.y - 0.5) * BeamWidth;
    float3 worldPos = beamPos + right * offset;
    
    
    output.position = mul(float4(worldPos, 1.0), ViewProjection);
    output.uv = input.uv;
    output.worldPos = worldPos;
    return output;
}