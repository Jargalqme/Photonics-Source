#include "pch.h"
#include "BeamWeapon.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

BeamWeapon::BeamWeapon(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

void BeamWeapon::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // CREATE CONSTANT BUFFER
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // LOAD SHADERS
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BeamVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BeamPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

    // CREATE RENDER STATES
    // Blend state (additive)
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

    // Depth stencil state (read-only depth)
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX::ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, m_depthStencilState.ReleaseAndGetAddressOf()));

    // Rasterizer state (no culling)
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = TRUE;
    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc, m_rasterizerState.ReleaseAndGetAddressOf()));
}

void BeamWeapon::update(float deltaTime)
{
    // always update time for animation
    m_time += deltaTime;

    // update cooldown
    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= deltaTime;
        if (m_cooldownTimer < 0.0f)
            m_cooldownTimer = 0.0f;
    }

    // update collision timer
    if (m_collisionLife > 0.0f)
    {
        m_collisionLife -= deltaTime;
        if (m_collisionLife < 0.0f)
            m_collisionLife = 0.0f;
    }

    // update beam lifetime
    if (m_isActive)
    {
        m_life -= deltaTime;
        if (m_life <= 0.0f)
        {
            m_isActive = false;
        }
    }
}

void BeamWeapon::fire(const Vector3& startPosition, const Vector3& direction)
{
    // check cooldown
    if (m_cooldownTimer > 0.0f) return;

    m_start = startPosition;
    // calculate end point
    Vector3 normalizedDir = direction;
    normalizedDir.Normalize();
    m_end = startPosition + normalizedDir * m_maxRange;

    // reset lifetime
    m_life = m_maxLife + 0.1f;
    m_isActive = true;

    // reset collision window
    m_collisionLife = m_collisionDuration;

    // start cooldown
    m_cooldownTimer = m_cooldown;
}

float BeamWeapon::getCooldownProgress() const
{
    if (m_cooldown <= 0.0f) return 0.0f;
    return m_cooldownTimer / m_cooldown;
}

void BeamWeapon::render(
    const Matrix& view,
    const Matrix& projection,
    const Vector3& cameraPosition)
{
    // Don't render if not active
    if (!m_isActive) return;

    auto context = m_deviceResources->GetD3DDeviceContext();

    // Update constant buffer
    ConstantBuffer cb = {};
    cb.viewProjection = (view * projection).Transpose();  // HLSL expects column-majorx
    cb.beamStart = m_start;
    cb.beamWidth = m_width;
    cb.beamEnd = m_end;
    cb.beamLife = m_life / m_maxLife;
    cb.beamColor = Vector4(m_color.R(), m_color.G(), m_color.B(), m_color.A());
    cb.cameraPosition = cameraPosition;
    cb.time = m_time;

    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // Set pipeline state
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(nullptr); // Fetch Shader を無効化

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // Set render states
    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    context->Draw(6, 0);

    // Reset states (same pattern as your Track)
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

void BeamWeapon::finalize()
{
    m_constantBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_blendState.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}

