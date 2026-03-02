#include "pch.h"
#include "MenuBackground.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

MenuBackground::MenuBackground(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_time(0.0f)
    , m_vertexCount(0)
{
}

void MenuBackground::initialize()
{
    createFullscreenQuad();
    createDeviceDependentResources();
}

void MenuBackground::update(float deltaTime)
{
    // Accumulate time for shader animation
    m_time += deltaTime;
}

void MenuBackground::createFullscreenQuad()
{
    m_vertices.clear();

    // Triangle 1: Top-left, Bottom-left, Top-right
    m_vertices.push_back({ XMFLOAT3(-1.0f,  1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) });  // Top-left
    m_vertices.push_back({ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) });  // Bottom-left
    m_vertices.push_back({ XMFLOAT3(1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) });   // Top-right

    // Triangle 2: Top-right, Bottom-left, Bottom-right
    m_vertices.push_back({ XMFLOAT3(1.0f,  1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) });   // Top-right
    m_vertices.push_back({ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) });  // Bottom-left
    m_vertices.push_back({ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) });   // Bottom-right

    m_vertexCount = m_vertices.size();
}

void MenuBackground::createDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Create Vertex Buffer
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(MenuVertex) * m_vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = m_vertices.data();

    DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, &initData, m_vertexBuffer.ReleaseAndGetAddressOf()));

    // Create Constant Buffer
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(MenuConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

    // Create shaders
    createShaders();

    // Blend State (Alpha blending)
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    DX::ThrowIfFailed(device->CreateBlendState(&blendDesc, m_blendState.ReleaseAndGetAddressOf()));

    // Depth Stencil State (Disable depth for 2D overlay)
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE;       // No depth testing
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.StencilEnable = FALSE;

    DX::ThrowIfFailed(device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.ReleaseAndGetAddressOf()));

    // Rasterizer State (No culling for fullscreen quad)
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthClipEnable = TRUE;

    DX::ThrowIfFailed(device->CreateRasterizerState(&rsDesc, m_rasterizerState.ReleaseAndGetAddressOf()));
}

void MenuBackground::createShaders()
{
    auto device = m_deviceResources->GetD3DDevice();

    // === Load Vertex Shader ===
    ComPtr<ID3DBlob> vertexShaderBlob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"MenuBackgroundVS.cso").c_str(), vertexShaderBlob.GetAddressOf()));
    
    DX::ThrowIfFailed(device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.ReleaseAndGetAddressOf()
    ));

    // Load Pixel Shader
    // Pixel shader 1: Warping (original)
    ComPtr<ID3DBlob> ps1Blob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"WarpingPS.cso").c_str(), ps1Blob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        ps1Blob->GetBufferPointer(),
        ps1Blob->GetBufferSize(),
        nullptr,
        m_pixelShaderWarping.ReleaseAndGetAddressOf()
    ));

    // Pixel shader 2: Fractal (Kishimisu)
    ComPtr<ID3DBlob> ps2Blob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"FractalPS.cso").c_str(), ps2Blob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        ps2Blob->GetBufferPointer(),
        ps2Blob->GetBufferSize(),
        nullptr,
        m_pixelShaderFractal.ReleaseAndGetAddressOf()
    ));

    // Pixel shader 3: Wave
    ComPtr<ID3DBlob> ps3Blob;
    DX::ThrowIfFailed(D3DReadFileToBlob(GetShaderPath(L"WavePS.cso").c_str(), ps3Blob.GetAddressOf()));
    DX::ThrowIfFailed(device->CreatePixelShader(
        ps3Blob->GetBufferPointer(),
        ps3Blob->GetBufferSize(),
        nullptr,
        m_pixelShaderWave.ReleaseAndGetAddressOf()
    ));

    // Create Input Layout
    static const D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(
        inputElements,
        ARRAYSIZE(inputElements),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        m_inputLayout.ReleaseAndGetAddressOf()
    ));
}

void MenuBackground::render()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    auto size = m_deviceResources->GetOutputSize();
    float width = static_cast<float>(size.right - size.left);
    float height = static_cast<float>(size.bottom - size.top);

    // Update Constant Buffer with all parameters
    MenuConstantBuffer cb;
    cb.Time = m_time;
    cb.Resolution = XMFLOAT2(width, height);
    cb.Speed = m_speed;
    cb.PatternScale = m_patternScale;
    cb.WarpIntensity = m_warpIntensity;
    cb.Brightness = m_brightness;
    cb.ChromaticOffset = m_chromaticOffset;
    cb.ColorTint = m_colorTint;
    cb.VignetteStrength = m_vignetteStrength;

    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

    // Set Pipeline State
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(MenuVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    // Select pixel shader based on type
    if (m_shaderType == ShaderType::Warping)
        context->PSSetShader(m_pixelShaderWarping.Get(), nullptr, 0);
    else if (m_shaderType == ShaderType::Fractal)
        context->PSSetShader(m_pixelShaderFractal.Get(), nullptr, 0);
    else
        context->PSSetShader(m_pixelShaderWave.Get(), nullptr, 0);

    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // Set render states (no depth, alpha blending)
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    context->RSSetState(m_rasterizerState.Get());

    // Draw fullscreen quad
    context->Draw(static_cast<UINT>(m_vertexCount), 0);

    // Reset states
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);
}

void MenuBackground::setShaderType(ShaderType type)
{
    m_shaderType = type;
}

void MenuBackground::onDeviceLost()
{
    m_vertexBuffer.Reset();
    m_constantBuffer.Reset();
    m_vertexShader.Reset();

    m_pixelShaderWarping.Reset();
    m_pixelShaderFractal.Reset();
    m_pixelShaderWave.Reset();

    m_inputLayout.Reset();
    m_blendState.Reset();
    m_depthStencilState.Reset();
    m_rasterizerState.Reset();
}