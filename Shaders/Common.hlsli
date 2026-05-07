//---------------------------------------------------------------------------
//! @file   Common.hlsli
//! @brief  Shared constants and HDR utility functions
//---------------------------------------------------------------------------

#ifndef COMMON_HLSLI
#define COMMON_HLSLI

// Constants
#define EPSILON 1.0e-4
#define HALF_MAX 65504.0

// Return the maximum of three values
float Max3(float a, float b, float c)
{
    return max(max(a, b), c);
}

// Clamp HDR value within a safe range
float3 SafeHDR(float3 c)
{
    return min(c, HALF_MAX);
}

float4 SafeHDR(float4 c)
{
    return min(c, HALF_MAX);
}

#endif