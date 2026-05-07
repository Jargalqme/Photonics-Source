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

// === 初期化・終了 ===

void ColorMaskEffect::createDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // 頂点シェーダー読み込み
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"FullscreenTriangleVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.ReleaseAndGetAddressOf()));

    // ピクセルシェーダー読み込み
    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"ColorMaskPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.ReleaseAndGetAddressOf()));

    // 定数バッファ
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ColorMaskCB);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr,
        m_constantBuffer.ReleaseAndGetAddressOf()));

    // サンプラー
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc,
        m_sampler.ReleaseAndGetAddressOf()));
}

void ColorMaskEffect::createWindowSizeDependentResources()
{
    // 現在はウィンドウサイズに依存するリソースなし
}

void ColorMaskEffect::finalize()
{
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_constantBuffer.Reset();
    m_sampler.Reset();
}

// === 処理 ===

void ColorMaskEffect::setColorMask(float r, float g, float b)
{
    m_colorMask = Vector3(r, g, b);
}

void ColorMaskEffect::process(ID3D11ShaderResourceView* inputSRV,
    ID3D11RenderTargetView* outputRTV)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // 定数バッファ更新
    ColorMaskCB cb = {};
    cb.colorMask = m_colorMask;
    cb.padding = 0.0f;
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // レンダーターゲット設定
    context->OMSetRenderTargets(1, &outputRTV, nullptr);

    // ビューポート設定
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // テクスチャ・サンプラー・定数バッファ設定
    context->PSSetShaderResources(0, 1, &inputSRV);
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // フルスクリーン三角形描画（SV_VertexID、頂点バッファなし）
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(nullptr);
    context->Draw(3, 0);

    // SRVアンバインド（D3D警告防止）
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
}
