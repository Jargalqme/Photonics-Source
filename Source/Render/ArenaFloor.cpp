#include "pch.h"
#include "ArenaFloor.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

ArenaFloor::ArenaFloor(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

// === 初期化 ===

void ArenaFloor::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // 頂点（y=0.01 — グリッドの少し上）
    float half = m_size;
    Vertex vertices[4] = {
        { XMFLOAT3(-half, 0.01f, -half) },
        { XMFLOAT3(-half, 0.01f,  half) },
        { XMFLOAT3( half, 0.01f, -half) },
        { XMFLOAT3( half, 0.01f,  half) }
    };
    uint16_t indices[6] = { 0, 1, 2, 2, 1, 3 };

    // 頂点バッファ
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = { vertices };
    DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData,
        m_vertexBuffer.ReleaseAndGetAddressOf()));

    // インデックスバッファ
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = { indices };
    DX::ThrowIfFailed(device->CreateBuffer(&ibDesc, &ibData,
        m_indexBuffer.ReleaseAndGetAddressOf()));

    // 定数バッファ
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr,
        m_constantBuffer.ReleaseAndGetAddressOf()));

    // シェーダー読み込み
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(
        GetShaderPath(L"WaveWorldVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(
        GetShaderPath(L"WaveWorldPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

    // 入力レイアウト
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    DX::ThrowIfFailed(device->CreateInputLayout(
        layout, 1,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        m_inputLayout.ReleaseAndGetAddressOf()));

    // ブレンドステート（アルファブレンド）
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc,
        m_blendState.ReleaseAndGetAddressOf()));

    // 深度ステンシル（読み取り専用 — 床の下に沈まない）
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX::ThrowIfFailed(device->CreateDepthStencilState(&dsDesc,
        m_depthStencilState.ReleaseAndGetAddressOf()));

    // ラスタライザー（カリングなし）
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = TRUE;
    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc,
        m_rasterizerState.ReleaseAndGetAddressOf()));
}

// === 終了処理 ===

void ArenaFloor::finalize()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_constantBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_inputLayout.Reset();
    m_blendState.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}

// === 更新 ===

void ArenaFloor::update(float deltaTime)
{
    m_time += deltaTime;
}

// === 描画 ===

void ArenaFloor::render(const Matrix& view, const Matrix& projection)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // 定数バッファ更新
    Matrix world = Matrix::Identity;
    ConstantBuffer cb;
    XMStoreFloat4x4(&cb.worldViewProjection,
        (world * view * projection).Transpose());
    cb.time = m_time;
    cb.speed = m_speed;
    cb.brightness = m_brightness;
    cb.alpha = m_alpha;
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // パイプライン設定
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    context->DrawIndexed(6, 0, 0);

    // ステート復元
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}
