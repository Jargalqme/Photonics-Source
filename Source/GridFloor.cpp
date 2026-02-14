#include "pch.h"
#include "GridFloor.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GridFloor::GridFloor(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

void GridFloor::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // ============================================================
    // 1. CREATE GEOMETRY (Vertex + Index Buffers)
    // ============================================================
    float half = m_gridSize;  // half-extent
    Vertex vertices[4] = {
        { XMFLOAT3(-half, 0, -half) },  // 0: back-left
        { XMFLOAT3(-half, 0,  half) },  // 1: front-left
        { XMFLOAT3(half, 0, -half) },   // 2: back-right
        { XMFLOAT3(half, 0,  half) }    // 3: front-right
    };
    uint16_t indices[6] = { 
        0, 1, 2, 
        2, 1, 3 
    };

    // Vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = { vertices };
    DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.ReleaseAndGetAddressOf()));

    // Index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = { indices };
    DX::ThrowIfFailed(device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.ReleaseAndGetAddressOf()));

    // ============================================================
    // 2. CREATE CONSTANT BUFFER
    // ============================================================
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // ============================================================
    // 3. LOAD SHADERS
    // ============================================================
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"PristineGridVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"PristineGridPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

    // Input layout
    D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    DX::ThrowIfFailed(device->CreateInputLayout(inputDesc, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf()));

    // ============================================================
    // 4. CREATE RENDER STATES
    // ============================================================
    // Blend state (additive)
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc, m_blendState.ReleaseAndGetAddressOf()));

    // Depth stencil state (read-only depth)
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX::ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, m_depthStencilState.ReleaseAndGetAddressOf()));

    // Rasterizer state (no culling, depth bias)
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthBias = -100000;
    rsDesc.SlopeScaledDepthBias = -2.0f;
    rsDesc.DepthClipEnable = TRUE;
    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc, m_rasterizerState.ReleaseAndGetAddressOf()));
}

void GridFloor::update()
{
    // calculate beat pulse effect 
    float pulse = 1.0f + cosf(m_beatPulse * 6.28318f) * 0.5f;

    // update the final color based on pulse
    //m_finalColor = Color(
    //    m_lineColor.R() * pulse,
    //    m_lineColor.G() * pulse,
    //    m_lineColor.B() * pulse,
    //    m_lineColor.A()
    //);

    m_finalColor = Color(
        m_lineColor.R(),
        m_lineColor.G(),
        m_lineColor.B(),
        m_lineColor.A()
    );
}

void GridFloor::render(const Matrix& view, const Matrix& projection) {
    renderPlane(m_worldFloor, view, projection);
    renderPlane(m_worldBack, view, projection);
    renderPlane(m_worldFront, view, projection);
    renderPlane(m_worldLeft, view, projection);
    renderPlane(m_worldRight, view, projection);
}

void GridFloor::renderPlane(const Matrix& world, const Matrix& view, const Matrix& projection)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Update constant buffer
    ConstantBuffer cb;

    cb.worldViewProjection = (world * view * projection).Transpose();
    cb.gridParams = Vector4(m_lineWidthX, m_lineWidthY, m_gridScale, 0.0f);
    cb.lineColor = Vector4(m_finalColor.R(), m_finalColor.G(), m_finalColor.B(), m_finalColor.A());
    cb.baseColor = Vector4(m_baseColor.R(), m_baseColor.G(), m_baseColor.B(), m_baseColor.A());
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // Set pipeline state
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

    // Draw
    context->DrawIndexed(6, 0, 0);

    // Reset states
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

void GridFloor::finalize()
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