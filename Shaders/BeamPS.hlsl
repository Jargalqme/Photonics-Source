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

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    // ビーム中心からの距離 (0 = 中心、1 = 端)
    float distFromCenter = abs(input.uv.y - 0.5) * 2.0;
    
    // 幅の異なる3つのグローレイヤー
    float core      = 1.0 - smoothstep(0.0, 0.1, distFromCenter);
    float innerGlow = 1.0 - smoothstep(0.0, 0.4, distFromCenter);
    float outerGlow = 1.0 - smoothstep(0.0, 1.0, distFromCenter);
    
    // 白いコア + 赤色のグローレイヤー
    float3 coreColor = float3(1.0, 1.0, 1.0);
    float3 glowColor = BeamColor.rgb;
    float3 color = coreColor * core * 2.0 
                 + glowColor * innerGlow * 1.5 
                 + glowColor * outerGlow * 0.5;
    
    // パルスは色出力全体を乗算します
    float pulse = sin(input.uv.x * 30.0 - Time * 20.0) * 0.15 + 1.0;
    color *= pulse;
    
    // フェード
    float tipFade   = smoothstep(0.98, 0.85, input.uv.x);
    float startFade = smoothstep(0.0, 0.05, input.uv.x);
    float lifeFade = BeamLife;
    float alpha = outerGlow * tipFade * startFade * lifeFade;
    return float4(color * alpha, alpha);
}