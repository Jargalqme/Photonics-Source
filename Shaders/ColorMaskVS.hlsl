struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Fullscreen triangle (GDC Vault 2014)
VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    // generate clip space position
    output.position.x = (float) (id / 2) * 4.0 - 1.0;
    output.position.y = (float) (id % 2) * 4.0 - 1.0;
    output.position.z = 0.0;
    output.position.w = 1.0;

    // texture coord
    output.uv.x = (float) (id / 2) * 2.0;
    output.uv.y = 1.0 - (float) (id % 2) * 2.0;

    return output;
}

// Alternative bit-trick version
//VSOutput main(uint vertexID : SV_VertexID)
//{
//    VSOutput output;
//    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
//    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
//    return output;
//}
