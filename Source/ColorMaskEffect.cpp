#include "pch.h"
#include "ColorMaskEffect.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

ColorMaskEffect::ColorMaskEffect(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_colorMask(1.0f, 1.0f, 1.0f)
{
}

void ColorMaskEffect::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Load vertex shader
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"ColorMaskVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.ReleaseAndGetAddressOf()));

    // Load pixel shader
    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"ColorMaskPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.ReleaseAndGetAddressOf()));

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ColorMaskCB);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr,
        m_constantBuffer.ReleaseAndGetAddressOf()));

    // Create sampler
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc,
        m_sampler.ReleaseAndGetAddressOf()));
}

void ColorMaskEffect::CreateWindowSizeDependentResources()
{
    // todo
}

void ColorMaskEffect::OnDeviceLost()
{
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_constantBuffer.Reset();
    m_sampler.Reset();
}

void ColorMaskEffect::SetColorMask(float r, float g, float b)
{
    m_colorMask = Vector3(r, g, b);
}

void ColorMaskEffect::Process(ID3D11ShaderResourceView* inputSRV,
    ID3D11RenderTargetView* outputRTV)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Update constant buffer with current mask
    ColorMaskCB cb = {};
    cb.colorMask = m_colorMask;
    cb.padding = 0.0f;
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0); // send colorMask to GPU

    // Set render target
    context->OMSetRenderTargets(1, &outputRTV, nullptr);                       // where to draw (backbuffer)

    // Set viewport
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Set shaders
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);                    // use shaders
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // Set texture and sampler
    context->PSSetShaderResources(0, 1, &inputSRV);                            // input texture (scene)
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());                    // how to sample texture
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());      // send constant buffer

    // Set topology (triangle)
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    // drawing triangles
    context->IASetInputLayout(nullptr);  // No input layout needed, no vertex buffer, using SV_VertexID

    // Draw fullscreen triangle (3 vertices, no vertex buffer)
    context->Draw(3, 0);                                                       // draw 3 vertices = 1 triangle (fullscreen)

    // Unbind SRV to prevent warnings
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);                             // prevent D3D warnings
}