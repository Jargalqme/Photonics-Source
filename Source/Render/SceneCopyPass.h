//=============================================================================
// @brief    Final composite step: samples a source texture and writes it to
//           the destination render target. Used to land the HDR scene (or
//           bloom output) into the BGRA backbuffer.
//
//           Replaces ColorMaskEffect, which was misnamed ? its color-mask
//           feature was never used in gameplay; the underlying scene-copy
//           was the load-bearing work.
//=============================================================================
#pragma once

#include "Core/DeviceResources.h"

class SceneCopyPass
{
public:
    SceneCopyPass(DX::DeviceResources* deviceResources);
    ~SceneCopyPass() = default;

    void createDeviceDependentResources();
    void finalize();

    void process(
        ID3D11ShaderResourceView* inputSRV,
        ID3D11RenderTargetView* outputRTV);

private:
    DX::DeviceResources* m_deviceResources;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
};