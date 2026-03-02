#include "pch.h"
#include "DeathBeamPool.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

DeathBeamPool::DeathBeamPool(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_color(0.0f, 0.0f, 0.0f, 0.0f)
    , m_beamHeight(20.0f)
    , m_beamWidth(5.0f)
    , m_duration(0.5f)
    , m_time(0.0f)
{
}

void DeathBeamPool::initialize(size_t poolSize)
{
    m_beams.resize(poolSize);
    for (auto& beam : m_beams)
    {
        beam.active = false;
        beam.life = 0.0f;
    }

    createDeviceDependentResources();
}

void DeathBeamPool::update(float deltaTime)
{
    m_time += deltaTime;

    for (auto& beam : m_beams)
    {
        if (beam.active)
        {
            beam.life -= deltaTime;
            if (beam.life <= 0.0f)
            {
                beam.active = false;
            }
        }
    }
}

void DeathBeamPool::trigger(const Vector3& position)
{
    // Find inactive beam
    for (auto& beam : m_beams)
    {
        if (!beam.active)
        {
            beam.position = position;
            beam.life = m_duration;
            beam.active = true;
            return;
        }
    }
}

void DeathBeamPool::createDeviceDependentResources()
{
    createGeometry();
    createShaders();

    auto device = m_deviceResources->GetD3DDevice();

    // Constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(BeamConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr,
        m_constantBuffer.ReleaseAndGetAddressOf()));

    // Additive blend state
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc,
        m_blendState.ReleaseAndGetAddressOf()));

    // Depth state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    DX::ThrowIfFailed(device->CreateDepthStencilState(&dsDesc,
        m_depthStencilState.ReleaseAndGetAddressOf()));

    // Rasterizer state
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = TRUE;

    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc,
        m_rasterizerState.ReleaseAndGetAddressOf()));
}

void DeathBeamPool::createGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();

    BeamVertex vertices[] =
    {
        { XMFLOAT3(0, 0, 0), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(0, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(0, 0, 0), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(0, 0, 0), XMFLOAT2(0.0f, 1.0f) },
        { XMFLOAT3(0, 0, 0), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(0, 0, 0), XMFLOAT2(1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData,
        m_vertexBuffer.ReleaseAndGetAddressOf()));
}

void DeathBeamPool::createShaders()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Reuse existing beam shaders
    ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BeamVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.ReleaseAndGetAddressOf()));

    ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BeamPS.cso").c_str(), psBlob.GetAddressOf()));

    DX::ThrowIfFailed(device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.ReleaseAndGetAddressOf()));

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(
        inputElements,
        ARRAYSIZE(inputElements),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_inputLayout.ReleaseAndGetAddressOf()));
}

void DeathBeamPool::render(
    const Matrix& view,
    const Matrix& projection,
    const Vector3& cameraPosition)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Set pipeline state once
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(BeamVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    // Render each active beam
    for (const auto& beam : m_beams)
    {
        if (!beam.active) continue;

        // Vertical beam: start at enemy position, end above
        Vector3 start = beam.position;
        Vector3 end = beam.position + Vector3(0.0f, m_beamHeight, 0.0f);

        BeamConstantBuffer cb = {};
        cb.viewProjection = (view * projection).Transpose();
        cb.beamStart = start;
        cb.beamWidth = m_beamWidth;
        cb.beamEnd = end;
        cb.beamLife = beam.life / m_duration;  // Normalized 0-1
        cb.beamColor = Vector4(m_color.R(), m_color.G(), m_color.B(), m_color.A());
        cb.cameraPosition = cameraPosition;
        cb.time = m_time;

        context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
        context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

        context->Draw(6, 0);
    }

    // Reset states
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

void DeathBeamPool::onDeviceLost()
{
    m_vertexBuffer.Reset();
    m_constantBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_inputLayout.Reset();
    m_blendState.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}