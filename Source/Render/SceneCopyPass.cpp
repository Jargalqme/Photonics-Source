#include "pch.h"
#include "Render/SceneCopyPass.h"

using Microsoft::WRL::ComPtr;

SceneCopyPass::SceneCopyPass(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

void SceneCopyPass::createDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Shared fullscreen-triangle VS (SV_VertexID-driven, no vertex buffer).
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"FullscreenTriangleVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.ReleaseAndGetAddressOf()));

    // Pass-through PS: samples input and writes to output.
    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"PS_SceneCopy.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.ReleaseAndGetAddressOf()));

    // Linear clamp sampler. Backbuffer copy never wraps.
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc,
        m_sampler.ReleaseAndGetAddressOf()));
}

void SceneCopyPass::finalize()
{
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_sampler.Reset();
}

void SceneCopyPass::process(
    ID3D11ShaderResourceView* inputSRV,
    ID3D11RenderTargetView* outputRTV)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    context->OMSetRenderTargets(1, &outputRTV, nullptr);

    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetShaderResources(0, 1, &inputSRV);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    // Fullscreen triangle: 3 vertices, SV_VertexID-driven, no IA buffer/layout.
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(nullptr);
    context->Draw(3, 0);

    // Unbind SRV so the next frame can rebind it as RT without D3D warnings.
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
}