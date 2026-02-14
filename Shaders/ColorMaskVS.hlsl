struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
    
    // Explicit vertex positions
    //float2 positions[3] =
    //{
    //    float2(-1,  1), // Top-left
    //    float2( 3,  1), // Far right
    //    float2(-1, -3)  // Far bottom
    //};

    //float2 uvs[3] =
    //{
    //    float2(0, 0),
    //    float2(2, 0),
    //    float2(0, 2)
    //};

    //VSOutput output;
    //output.position = float4(positions[vertexID], 0, 1);
    //output.uv = uvs[vertexID];
    //return output;
}