#include "pch.h"
#include "Skybox.h"
#include "WICTextureLoader.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Skybox::Skybox(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

// === 初期化・終了 ===

void Skybox::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // 6面テクスチャ読み込み
    // DX11キューブ面順序: +X(右), -X(左), +Y(上), -Y(下), +Z(前), -Z(後)
    const wchar_t* faceFiles[6] = {
        L"Assets/Textures/skybox_right.png",
        L"Assets/Textures/skybox_left.png",
        L"Assets/Textures/skybox_top.png",
        L"Assets/Textures/skybox_bottom.png",
        L"Assets/Textures/skybox_front.png",
        L"Assets/Textures/skybox_back.png",
    };

    ComPtr<ID3D11Texture2D> faceTextures[6];

    for (uint32_t i = 0; i < 6; i++)
    {
        ComPtr<ID3D11Resource> resource;
        DX::ThrowIfFailed(CreateWICTextureFromFile(
            device, faceFiles[i], resource.GetAddressOf(), nullptr));
        DX::ThrowIfFailed(resource.As(&faceTextures[i]));
    }

    // 最初の面からフォーマットとサイズを取得
    D3D11_TEXTURE2D_DESC faceDesc;
    faceTextures[0]->GetDesc(&faceDesc);

    // キューブテクスチャ作成
    D3D11_TEXTURE2D_DESC cubeDesc = {};
    cubeDesc.Width = faceDesc.Width;
    cubeDesc.Height = faceDesc.Height;
    cubeDesc.MipLevels = 1;
    cubeDesc.ArraySize = 6;
    cubeDesc.Format = faceDesc.Format;
    cubeDesc.SampleDesc.Count = 1;
    cubeDesc.Usage = D3D11_USAGE_DEFAULT;
    cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    ComPtr<ID3D11Texture2D> cubeTexture;
    DX::ThrowIfFailed(device->CreateTexture2D(&cubeDesc, nullptr, cubeTexture.GetAddressOf()));

    // 各面をキューブテクスチャにコピー
    for (uint32_t i = 0; i < 6; i++)
    {
        uint32_t destSubresource = D3D11CalcSubresource(0, i, 1);
        context->CopySubresourceRegion(
            cubeTexture.Get(), destSubresource, 0, 0, 0,
            faceTextures[i].Get(), 0, nullptr);
    }

    // SRV作成（TextureCube）
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = faceDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;
    DX::ThrowIfFailed(device->CreateShaderResourceView(
        cubeTexture.Get(), &srvDesc, m_cubeSRV.GetAddressOf()));

    // サンプラー（線形フィルタリング、クランプ）
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_sampler.GetAddressOf()));

    // シェーダー読み込み
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(
        GetShaderPath(L"SkyboxVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(
        GetShaderPath(L"SkyboxPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

    // 定数バッファ
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(
        &cbDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // 深度ステンシル：テストなし、書き込みなし
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DX::ThrowIfFailed(device->CreateDepthStencilState(
        &dsDesc, m_depthStencilState.ReleaseAndGetAddressOf()));

    // ラスタライザー：カリングなし
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = FALSE;
    DX::ThrowIfFailed(device->CreateRasterizerState(
        &rsDesc, m_rasterizerState.ReleaseAndGetAddressOf()));
}

void Skybox::finalize()
{
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_constantBuffer.Reset();
    m_cubeSRV.Reset();
    m_sampler.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}

// === 描画 ===

void Skybox::render(const Matrix& view, const Matrix& proj)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // 平行移動を除去 — スカイボックスはカメラに追従
    Matrix viewRotOnly = view;
    viewRotOnly._41 = 0.0f;
    viewRotOnly._42 = 0.0f;
    viewRotOnly._43 = 0.0f;

    Matrix invVP = (viewRotOnly * proj).Invert();

    ConstantBuffer cb;
    cb.inverseViewProjection = invVP.Transpose();
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // パイプライン設定
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // キューブマップテクスチャとサンプラーをバインド
    context->PSSetShaderResources(0, 1, m_cubeSRV.GetAddressOf());
    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    // フルスクリーン三角形描画
    context->Draw(3, 0);

    // ステート復元
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}
