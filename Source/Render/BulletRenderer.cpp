#include "pch.h"
#include "BulletRenderer.h"
#include "Gameplay/BulletPool.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

BulletRenderer::BulletRenderer(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

// === 初期化 ===

void BulletRenderer::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // 動的構造化バッファ（弾データ用）
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(BoltData) * MAX_BOLTS;
    bufDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(BoltData);
    DX::ThrowIfFailed(device->CreateBuffer(&bufDesc, nullptr, m_boltBuffer.ReleaseAndGetAddressOf()));

    // SRV（頂点シェーダーから読み取り用）
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = MAX_BOLTS;
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_boltBuffer.Get(), &srvDesc, m_boltSRV.ReleaseAndGetAddressOf()));

    // 定数バッファ
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(BoltCB);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // シェーダー読み込み
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BoltVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BeamBoltPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_playerPS.ReleaseAndGetAddressOf()));

    ComPtr<ID3DBlob> orbVsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"OrbVS.cso").c_str(), orbVsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(orbVsBlob->GetBufferPointer(), orbVsBlob->GetBufferSize(), nullptr, m_orbVertexShader.ReleaseAndGetAddressOf()));

    ComPtr<ID3DBlob> orbPsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"OrbPS.cso").c_str(), orbPsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(orbPsBlob->GetBufferPointer(), orbPsBlob->GetBufferSize(), nullptr, m_enemyPS.ReleaseAndGetAddressOf()));

    // レンダーステート（加算ブレンド、深度読み取りのみ、カリングなし）
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc, m_blendState.ReleaseAndGetAddressOf()));

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX::ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, m_depthStencilState.ReleaseAndGetAddressOf()));

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = TRUE;
    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc, m_rasterizerState.ReleaseAndGetAddressOf()));
}

// === 更新 ===

void BulletRenderer::update(float deltaTime)
{
    m_time += deltaTime;
}

// === 描画 ===

void BulletRenderer::render(
    BulletPool* bulletPool,
    const Matrix& view,
    const Matrix& projection,
    const Vector3& cameraPosition)
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    Bullet* bullets = bulletPool->getBullets();
    int maxBullets = bulletPool->getMaxBullets();

    // 共通パイプライン設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(nullptr);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->VSSetShaderResources(0, 1, m_boltSRV.GetAddressOf());

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    // --- パス1: プレイヤー弾（シアン、コメット軌跡） ---
    {
        uint32_t boltCount = 0;

        for (int i = 0; i < maxBullets && boltCount < MAX_BOLTS; i++)
        {
            if (bullets[i].isActive() && bullets[i].getFaction() == CombatFaction::Player)
            {
                BoltData& b = m_boltData[boltCount];
                b.position = bullets[i].getPosition();
                b.speed = bullets[i].getMaxSpeed();
                b.direction = bullets[i].getDirection();
                b.age = bullets[i].getAge();
                b.color = Vector4(0.0f, 0.85f, 1.0f, 1.0f);
                boltCount++;
            }
        }

        if (boltCount > 0)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            DX::ThrowIfFailed(context->Map(m_boltBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
            memcpy(mapped.pData, m_boltData, sizeof(BoltData) * boltCount);
            context->Unmap(m_boltBuffer.Get(), 0);

            BoltCB cb = {};
            cb.viewProjection = (view * projection).Transpose();
            cb.cameraPosition = cameraPosition;
            cb.time = m_time;
            cb.boltLength = PLAYER_BOLT_LENGTH;
            cb.boltWidth = PLAYER_BOLT_WIDTH;
            cb.boltCount = boltCount;
            context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

            context->PSSetShader(m_playerPS.Get(), nullptr, 0);
            context->Draw(VERTS_PER_BOLT * boltCount, 0);
        }
    }

    // --- パス2: 敵弾（赤橙、発光オーブ） ---
    {
        uint32_t boltCount = 0;

        for (int i = 0; i < maxBullets && boltCount < MAX_BOLTS; i++)
        {
            if (bullets[i].isActive() && bullets[i].getFaction() == CombatFaction::Enemy)
            {
                BoltData& b = m_boltData[boltCount];
                b.position = bullets[i].getPosition();
                b.speed = bullets[i].getMaxSpeed();
                b.direction = bullets[i].getDirection();
                b.age = bullets[i].getAge();
                b.color = bullets[i].getColor();
                boltCount++;
            }
        }

        if (boltCount > 0)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            DX::ThrowIfFailed(context->Map(m_boltBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
            memcpy(mapped.pData, m_boltData, sizeof(BoltData) * boltCount);
            context->Unmap(m_boltBuffer.Get(), 0);

            BoltCB cb = {};
            cb.viewProjection = (view * projection).Transpose();
            cb.cameraPosition = cameraPosition;
            cb.time = m_time;
            cb.boltLength = 0.0f;
            cb.boltWidth = ENEMY_ORB_RADIUS;
            cb.boltCount = boltCount;
            context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

            // オーブシェーダーに切り替え
            context->VSSetShader(m_orbVertexShader.Get(), nullptr, 0);
            context->PSSetShader(m_enemyPS.Get(), nullptr, 0);
            context->Draw(VERTS_PER_ORB * boltCount, 0);

            // 次フレーム用にボルトVSを復元
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        }
    }

    // SRV 解除
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->VSSetShaderResources(0, 1, &nullSRV);

    // ステート復元
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

// === 終了処理 ===

void BulletRenderer::finalize()
{
    m_boltBuffer.Reset();
    m_boltSRV.Reset();
    m_constantBuffer.Reset();
    m_vertexShader.Reset();
    m_orbVertexShader.Reset();
    m_playerPS.Reset();
    m_enemyPS.Reset();
    m_blendState.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}
