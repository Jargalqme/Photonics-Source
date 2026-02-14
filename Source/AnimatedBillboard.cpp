#include "pch.h"
#include "AnimatedBillboard.h"
#include "WICTextureLoader.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

AnimatedBillboard::AnimatedBillboard(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

void AnimatedBillboard::initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // 1. CREATE GEOMETRY (Vertex + Index Buffers)
    Vertex vertices[4] =
    {
        { XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2(0.0f, 1.0f) },  // bottom-left
        { XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f) },  // top-left
        { XMFLOAT3(0.5f,  0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f) },  // top-right
        { XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 1.0f) },  // bottom-right
    };
    uint16_t indices[6] = { 
        0, 1, 2, 
        0, 2, 3 
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

    // 2. CREATE CONSTANT BUFFER
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // 3. LOAD SHADERS
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BillboardVS.cso").c_str(), vsBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"BillboardPS.cso").c_str(), psBlob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

    // Input layout
    D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    DX::ThrowIfFailed(device->CreateInputLayout(inputDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf()));

    // 4. LOAD TEXTURE
    ComPtr<ID3D11Resource> texResource;
    DX::ThrowIfFailed(CreateWICTextureFromFile(device, GetAssetPath(L"Textures/spritesheet.png").c_str(),
        texResource.GetAddressOf(),
        m_textureSRV.ReleaseAndGetAddressOf()));

    // Sampler state (linear filtering, clamp)
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.ReleaseAndGetAddressOf()));

    // 5. CREATE RENDER STATES
    // Blend state (alpha blending)
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
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

void AnimatedBillboard::update(float deltaTime)
{
    m_frameTimer += deltaTime;
    float frameDuration = 1.0f / m_framesPerSecond;

    if (m_frameTimer >= frameDuration)
    {
        m_frameTimer -= frameDuration;
        m_currentFrame = (m_currentFrame + 1) % 36;
    }

    int col = m_currentFrame % 6;
    int row = m_currentFrame / 6;
    m_frameOffsetU = col / 6.0f;
    m_frameOffsetV = row / 6.0f;
}

void AnimatedBillboard::render(
    const Matrix& view,
    const Matrix& projection,
    const Vector3& cameraPosition)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Extract camera right and up from inverse view matrix
    Matrix invView = view.Invert();
    Vector3 cameraRight(invView._11, invView._12, invView._13);
    Vector3 cameraUp(invView._21, invView._22, invView._23);

    // Update constant buffer
    ConstantBuffer cb = {};
    cb.viewProjection = (view * projection).Transpose();  // HLSL expects column-major
    cb.worldPosition = m_position;
    cb.billboardSize = m_size;
    cb.cameraRight = cameraRight;
    cb.frameOffsetU = m_frameOffsetU;
    cb.cameraUp = cameraUp;
    cb.frameOffsetV = m_frameOffsetV;
    cb.frameSizeU = 1.0f / 6.0f;
    cb.frameSizeV = 1.0f / 6.0f;
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

    context->PSSetShaderResources(0, 1, m_textureSRV.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    // Set render states
    float blendFactor[4] = { 0, 0, 0, 0 };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    context->DrawIndexed(6, 0, 0);

    // Reset states (same pattern as BeamWeapon)
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
    ID3D11ShaderResourceView* nullSRV = nullptr;
    context->PSSetShaderResources(0, 1, &nullSRV);
}

void AnimatedBillboard::finalize()
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
    m_textureSRV.Reset();
    m_samplerState.Reset();
}