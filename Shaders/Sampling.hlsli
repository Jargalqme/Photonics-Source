//---------------------------------------------------------------------------
//! @file   Sampling.hlsli
//! @brief  Bloom sampling functions (Jimenez 2014 CoD AW technique)
//---------------------------------------------------------------------------

#ifndef SAMPLING_HLSLI
#define SAMPLING_HLSLI

#include "Common.hlsli"

// 13-tap box downsample (Jimenez 2014)
// Weighted sampling pattern that avoids pulsating artifacts
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
float4 DownsampleBox13Tap(Texture2D tex, SamplerState samp, float2 uv, float2 texelSize)
{
    float4 A = tex.Sample(samp, uv + texelSize * float2(-1.0, -1.0));
    float4 B = tex.Sample(samp, uv + texelSize * float2(0.0, -1.0));
    float4 C = tex.Sample(samp, uv + texelSize * float2(1.0, -1.0));
    float4 D = tex.Sample(samp, uv + texelSize * float2(-0.5, -0.5));
    float4 E = tex.Sample(samp, uv + texelSize * float2(0.5, -0.5));
    float4 F = tex.Sample(samp, uv + texelSize * float2(-1.0, 0.0));
    float4 G = tex.Sample(samp, uv);
    float4 H = tex.Sample(samp, uv + texelSize * float2(1.0, 0.0));
    float4 I = tex.Sample(samp, uv + texelSize * float2(-0.5, 0.5));
    float4 J = tex.Sample(samp, uv + texelSize * float2(0.5, 0.5));
    float4 K = tex.Sample(samp, uv + texelSize * float2(-1.0, 1.0));
    float4 L = tex.Sample(samp, uv + texelSize * float2(0.0, 1.0));
    float4 M = tex.Sample(samp, uv + texelSize * float2(1.0, 1.0));

    float2 div = (1.0 / 4.0) * float2(0.5, 0.125);

    float4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

// 9-tap bilinear upsample (tent filter)
float4 UpsampleTent(Texture2D tex, SamplerState samp, float2 uv, float2 texelSize, float sampleScale)
{
    float4 d = texelSize.xyxy * float4(1.0, 1.0, -1.0, 0.0) * sampleScale;

    float4 s;
    s = tex.Sample(samp, uv - d.xy);
    s += tex.Sample(samp, uv - d.wy) * 2.0;
    s += tex.Sample(samp, uv - d.zy);

    s += tex.Sample(samp, uv + d.zw) * 2.0;
    s += tex.Sample(samp, uv) * 4.0;
    s += tex.Sample(samp, uv + d.xw) * 2.0;

    s += tex.Sample(samp, uv + d.zy);
    s += tex.Sample(samp, uv + d.wy) * 2.0;
    s += tex.Sample(samp, uv + d.xy);

    return s * (1.0 / 16.0);
}

// Quadratic color thresholding (soft knee)
// curve = (threshold - knee, knee * 2, 0.25 / knee)
float4 QuadraticThreshold(float4 color, float threshold, float3 curve)
{
    float br = Max3(color.r, color.g, color.b);

    float rq = clamp(br - curve.x, 0.0, curve.y);
    rq = curve.z * rq * rq;

    color *= max(rq, br - threshold) / max(br, EPSILON);

    return color;
}

#endif