  cbuffer BillboardConstants : register(b0)
{
    matrix ViewProjection;
    float3 WorldPosition;
    float BillboardSize;
    float3 CameraRight;
    float FrameOffsetU;
    float3 CameraUp;
    float FrameOffsetV;
    float FrameSizeU;
    float FrameSizeV;
    float2 Padding;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PSInput main(VSInput input)
{
    PSInput output;

    float3 worldPos = WorldPosition
          + CameraRight * (input.position.x * BillboardSize)
          + CameraUp * (input.position.y * BillboardSize);

    output.position = mul(float4(worldPos, 1.0), ViewProjection);

    output.uv = float2(
          FrameOffsetU + input.uv.x * FrameSizeU,
          FrameOffsetV + input.uv.y * FrameSizeV
      );

    return output;
}