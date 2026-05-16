#include "pch.h"
#include "Renderer.h"
#include "Source/Render/RenderCommandQueue.h"
#include "Source/Render/Billboard.h"
#include "Source/Render/ImportedModel.h"
#include "Source/Render/RenderUtil.h"

#include <cstddef>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

namespace
{
    struct ImportedModelCBData
    {
        Matrix worldViewProjection;
        Matrix worldInverseTranspose;
        Vector4 color;
        Vector4 lightDirectionAndAmbient;
        Vector4 materialFlags;
    };

    static_assert((sizeof(ImportedModelCBData) % 16) == 0);

    Vector4 CombineImportedModelColor(
        const ImportedModelCommand& command,
        const ImportedModel& model,
        const ImportedSubmesh& submesh)
    {
        Vector4 color(command.color.x, command.color.y, command.color.z, command.color.w);

        const auto& materials = model.materials();
        if (submesh.materialIndex < materials.size())
        {
            const Color& materialColor = materials[submesh.materialIndex].baseColor;
            color.x *= materialColor.x;
            color.y *= materialColor.y;
            color.z *= materialColor.z;
            color.w *= materialColor.w;
        }

        return color;
    }

    int32_t GetImportedModelBaseColorTextureIndex(
        const ImportedModel& model,
        const ImportedSubmesh& submesh)
    {
        const auto& materials = model.materials();
        if (submesh.materialIndex >= materials.size())
        {
            return IMPORTED_TEXTURE_NONE;
        }

        return materials[submesh.materialIndex].baseColorTextureIndex;
    }

    void SetImportedModelBlendState(
        ID3D11DeviceContext* context,
        const ImportedModelCommand& command,
        ID3D11BlendState* alphaBlendState,
        ID3D11BlendState* additiveBlendState)
    {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        switch (command.blendMode)
        {
        case BlendMode::AlphaBlend:
            context->OMSetBlendState(alphaBlendState, blendFactor, 0xFFFFFFFF);
            break;
        case BlendMode::Additive:
            context->OMSetBlendState(additiveBlendState, blendFactor, 0xFFFFFFFF);
            break;
        case BlendMode::Opaque:
        default:
            context->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
            break;
        }
    }
}

Renderer::Renderer(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_sceneRenderer(std::make_unique<SceneRenderer>(deviceResources))
{
}

void Renderer::Clear()
{
    // NOTE: This method is currently unused because BeginScene handles clearing.
    // It's kept for potential future use (debugging, alternate render paths, etc.)

	// Clear the views
	m_deviceResources->PIXBeginEvent(L"Clear");

	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	// Dark blue Tron-style background
    static const float darkTronBlue[4] = { 0.12f, 0.05f, 0.2f, 1.0f };

	context->ClearRenderTargetView(renderTarget, darkTronBlue);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	const auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	m_deviceResources->PIXEndEvent();
}

void Renderer::CreateDeviceDependentResources()
{
    m_sceneRenderer->createDeviceDependentResources();
    CreateImportedModelResources();
    CreateUIResources();
}

void Renderer::CreateWindowSizeDependentResources()
{
    // シーン HDR テクスチャ + ブルームはレンダー解像度ベース
    int renderW = GetRenderWidth();
    int renderH = GetRenderHeight();
    m_sceneRenderer->createWindowSizeDependentResources(renderW, renderH);

    auto device = m_deviceResources->GetD3DDevice();
    UINT width = static_cast<UINT>(renderW);
    UINT height = static_cast<UINT>(renderH);

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr,
        m_sceneTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateRenderTargetView(m_sceneTexture.Get(), nullptr,
        m_sceneRTV.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_sceneTexture.Get(), nullptr,
        m_sceneSRV.ReleaseAndGetAddressOf()));

    // シーン専用 DSV（レンダー解像度サイズ）— バックバッファ DSV とは別個に持つ。
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    DX::ThrowIfFailed(device->CreateTexture2D(&depthDesc, nullptr,
        m_sceneDepthTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateDepthStencilView(m_sceneDepthTexture.Get(), nullptr,
        m_sceneDSV.ReleaseAndGetAddressOf()));

    // UI スケーリングはバックバッファ（ウィンドウ）サイズ
    auto windowSize = m_deviceResources->GetOutputSize();
    m_screenWidth = static_cast<float>(windowSize.right - windowSize.left);
    m_screenHeight = static_cast<float>(windowSize.bottom - windowSize.top);
    UpdateScaling();
}

void Renderer::SetRenderResolution(int width, int height)
{
    if (width == m_renderWidth && height == m_renderHeight)
    {
        return;
    }
    m_renderWidth = width;
    m_renderHeight = height;
    CreateWindowSizeDependentResources();
}

int Renderer::GetRenderWidth() const
{
    if (m_renderWidth > 0)
    {
        return m_renderWidth;
    }
    auto size = m_deviceResources->GetOutputSize();
    return int(size.right - size.left);
}

int Renderer::GetRenderHeight() const
{
    if (m_renderHeight > 0)
    {
        return m_renderHeight;
    }
    auto size = m_deviceResources->GetOutputSize();
    return int(size.bottom - size.top);
}

void Renderer::ApplyPostProcess()
{
    m_sceneRenderer->renderPostProcess(
        m_sceneSRV.Get(),
        m_deviceResources->GetRenderTargetView());
}

void Renderer::OnDeviceLost()
{
    m_sceneRenderer->onDeviceLost();
    m_sceneTexture.Reset();
    m_sceneRTV.Reset();
    m_sceneSRV.Reset();
    m_sceneDepthTexture.Reset();
    m_sceneDSV.Reset();
    m_importedModelVS.Reset();
    m_importedModelPS.Reset();
    m_importedModelInputLayout.Reset();
    m_importedModelConstantBuffer.Reset();
    m_importedModelSolidRasterizer.Reset();
    m_importedModelWireframeRasterizer.Reset();
    m_importedModelSampler.Reset();
    m_importedModelAlphaBlendState.Reset();
    m_importedModelAdditiveBlendState.Reset();

    // Release UI resources
    m_spriteBatch.reset();
    m_fontTitle.reset(); 
    m_fontQuote.reset();  
    m_fontMenu.reset();   
    m_fontHud.reset();    
    m_whiteTexture.Reset();
}

void Renderer::BeginScene()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Scene color
    static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Get RTV as local variable (important)
    ID3D11RenderTargetView* rtv = m_sceneRTV.Get();
    ID3D11DepthStencilView* dsv = m_sceneDSV.Get();

    // Set render target — シーン RTV + シーン DSV（両方ともレンダー解像度）
    context->OMSetRenderTargets(1, &rtv, dsv);

    // Clear
    context->ClearRenderTargetView(rtv, clearColor);
    context->ClearDepthStencilView(dsv,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // ビューポートはレンダー解像度に合わせる（デフォルトはバックバッファサイズ）。
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width  = float(GetRenderWidth());
    viewport.Height = float(GetRenderHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
}

void Renderer::EndScene()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Unbind scene texture so we can read from it
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, nullptr);

    // Hand the HDR scene to SceneRenderer for post-process + backbuffer copy.
    ApplyPostProcess();
}

void Renderer::CreateImportedModelResources()
{
    auto* device = m_deviceResources->GetD3DDevice();

    ComPtr<ID3DBlob> vsBlob;
    m_importedModelVS = RenderUtil::loadVS(device, L"ImportedModelVS.cso", &vsBlob);
    m_importedModelPS = RenderUtil::loadPS(device, L"ImportedModelPS.cso");

    const D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ImportedModelVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ImportedModelVertex, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ImportedModelVertex, tangent),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(ImportedModelVertex, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(device->CreateInputLayout(
        inputElements,
        static_cast<UINT>(std::size(inputElements)),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_importedModelInputLayout.ReleaseAndGetAddressOf()));

    m_importedModelConstantBuffer = RenderUtil::createConstantBuffer<ImportedModelCBData>(device);

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;
    DX::ThrowIfFailed(device->CreateRasterizerState(
        &rasterizerDesc,
        m_importedModelSolidRasterizer.ReleaseAndGetAddressOf()));

    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    DX::ThrowIfFailed(device->CreateRasterizerState(
        &rasterizerDesc,
        m_importedModelWireframeRasterizer.ReleaseAndGetAddressOf()));

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    DX::ThrowIfFailed(device->CreateSamplerState(
        &samplerDesc,
        m_importedModelSampler.ReleaseAndGetAddressOf()));

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    DX::ThrowIfFailed(device->CreateBlendState(
        &blendDesc,
        m_importedModelAlphaBlendState.ReleaseAndGetAddressOf()));

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    DX::ThrowIfFailed(device->CreateBlendState(
        &blendDesc,
        m_importedModelAdditiveBlendState.ReleaseAndGetAddressOf()));
}

void Renderer::DrawImportedModelCommand(
    const ImportedModelCommand& command,
    const Matrix& view,
    const Matrix& projection)
{
    if (!command.model ||
        !command.model->vertexBuffer() ||
        !command.model->indexBuffer() ||
        !m_importedModelVS ||
        !m_importedModelPS ||
        !m_importedModelInputLayout ||
        !m_importedModelConstantBuffer ||
        !m_importedModelSampler)
    {
        return;
    }

    auto* context = m_deviceResources->GetD3DDeviceContext();
    const ImportedModel& model = *command.model;

    UINT stride = model.vertexStride();
    UINT offset = 0;
    ID3D11Buffer* vertexBuffer = model.vertexBuffer();
    ID3D11Buffer* constantBuffer = m_importedModelConstantBuffer.Get();

    context->IASetInputLayout(m_importedModelInputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(model.indexBuffer(), DXGI_FORMAT_R32_UINT, 0);

    context->VSSetShader(m_importedModelVS.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetShader(m_importedModelPS.Get(), nullptr, 0);
    context->PSSetConstantBuffers(0, 1, &constantBuffer);
    ID3D11SamplerState* sampler = m_importedModelSampler.Get();
    context->PSSetSamplers(0, 1, &sampler);

    ID3D11RasterizerState* rasterizer =
        command.wireframe ? m_importedModelWireframeRasterizer.Get() : m_importedModelSolidRasterizer.Get();
    context->RSSetState(rasterizer);

    Matrix normalWorld = command.world;
    normalWorld._41 = 0.0f;
    normalWorld._42 = 0.0f;
    normalWorld._43 = 0.0f;

    const Matrix normalMatrix = normalWorld.Invert().Transpose();

    ImportedModelCBData cb = {};
    cb.worldViewProjection = (command.world * view * projection).Transpose();
    cb.worldInverseTranspose = normalMatrix.Transpose();
    cb.lightDirectionAndAmbient = Vector4(-0.35f, -0.85f, 0.35f, 0.28f);

    const auto& submeshes = model.submeshes();
    if (submeshes.empty())
    {
        cb.color = Vector4(command.color.x, command.color.y, command.color.z, command.color.w);
        cb.materialFlags = Vector4::Zero;

        ID3D11ShaderResourceView* nullTexture = nullptr;
        context->PSSetShaderResources(0, 1, &nullTexture);

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        DX::ThrowIfFailed(context->Map(m_importedModelConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        *reinterpret_cast<ImportedModelCBData*>(mapped.pData) = cb;
        context->Unmap(m_importedModelConstantBuffer.Get(), 0);

        context->DrawIndexed(model.indexCount(), 0, 0);
        context->RSSetState(nullptr);
        return;
    }

    for (const auto& submesh : submeshes)
    {
        if (submesh.indexCount == 0)
        {
            continue;
        }

        cb.color = CombineImportedModelColor(command, model, submesh);
        const int32_t textureIndex = GetImportedModelBaseColorTextureIndex(model, submesh);
        ID3D11ShaderResourceView* textureSRV = model.textureSRV(textureIndex);
        cb.materialFlags = Vector4(textureSRV ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
        context->PSSetShaderResources(0, 1, &textureSRV);

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        DX::ThrowIfFailed(context->Map(m_importedModelConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        *reinterpret_cast<ImportedModelCBData*>(mapped.pData) = cb;
        context->Unmap(m_importedModelConstantBuffer.Get(), 0);

        context->DrawIndexed(submesh.indexCount, submesh.startIndex, 0);
    }

    ID3D11ShaderResourceView* nullTexture = nullptr;
    context->PSSetShaderResources(0, 1, &nullTexture);
    context->RSSetState(nullptr);
}

void Renderer::ExecuteRenderCommands(
    const RenderCommandQueue& queue,
    const Matrix& view,
    const Matrix& projection,
    const Vector3& cameraPosition)
{
    (void)cameraPosition;

    auto drawMesh = [&](const MeshCommand& command)
    {
        if (!command.mesh)
        {
            return;
        }

        command.mesh->Draw(
            command.world,
            view,
            projection,
            command.color,
            nullptr,
            command.wireframe);
    };

    for (const auto& command : queue.opaqueMeshes())
    {
        drawMesh(command);
    }

    auto* context = m_deviceResources->GetD3DDeviceContext();
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (const auto& command : queue.opaqueImportedModels())
    {
        context->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
        DrawImportedModelCommand(command, view, projection);
    }

    for (const auto& command : queue.transparentMeshes())
    {
        drawMesh(command);
    }

    for (const auto& command : queue.transparentImportedModels())
    {
        SetImportedModelBlendState(
            context,
            command,
            m_importedModelAlphaBlendState.Get(),
            m_importedModelAdditiveBlendState.Get());
        DrawImportedModelCommand(command, view, projection);
    }

    context->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);

    for (const auto& command : queue.transparentBillboards())
    {
        if (!command.billboard)
        {
            continue;
        }

        command.billboard->render(view, projection, command.position, command.size);
    }
}

void Renderer::BeginViewmodelPass()
{
    // 深度をクリアしてビューモデル（武器）を最前面に描画できるようにする。
    // シーン DSV を対象にする（レンダー解像度サイズ）。
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearDepthStencilView(
        m_sceneDSV.Get(),
        D3D11_CLEAR_DEPTH, 1.0f, 0);
}

// 2D / UI Implementation
void Renderer::CreateUIResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_spriteBatch = std::make_unique<SpriteBatch>(context);

    try
    {
        m_fontTitle = std::make_unique<SpriteFont>(device, GetAssetPath(L"Fonts/SegoeUI_64_Bold.spritefont").c_str());
        m_fontQuote = std::make_unique<SpriteFont>(device, GetAssetPath(L"Fonts/SegoeUI_42_Regular.spritefont").c_str());
        m_fontMenu = std::make_unique<SpriteFont>(device, GetAssetPath(L"Fonts/SegoeUI_32_Regular.spritefont").c_str());
        m_fontHud = std::make_unique<SpriteFont>(device, GetAssetPath(L"Fonts/SegoeUI_24_Regular.spritefont").c_str());
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA("Font loading failed: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }

    // Create 1x1 white texture for DrawRect
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    uint32_t whitePixel = 0xFFFFFFFF;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &whitePixel;
    initData.SysMemPitch = sizeof(uint32_t);

    ComPtr<ID3D11Texture2D> tex;
    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, &initData, tex.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateShaderResourceView(tex.Get(), nullptr,
        m_whiteTexture.ReleaseAndGetAddressOf()));
}

SpriteFont* Renderer::GetFont(FontType type)
{
    switch (type)
    {
    case FontType::Title: return m_fontTitle.get();
    case FontType::Quote: return m_fontQuote.get();
    case FontType::Menu:  return m_fontMenu.get();
    case FontType::Hud:   return m_fontHud.get();
    default:              return m_fontMenu.get();
    }
}

void Renderer::UpdateScaling()
{
    m_scaleX = m_screenWidth / REF_WIDTH;
    m_scaleY = m_screenHeight / REF_HEIGHT;
}

void Renderer::BeginUI()
{
    m_spriteBatch->Begin(SpriteSortMode_Deferred, nullptr);  // Default blend state (alpha)
}

void Renderer::EndUI()
{
    m_spriteBatch->End();
}

void Renderer::DrawRect(const RECT& refRect, FXMVECTOR color, float alpha)
{
    if (!m_whiteTexture) return;

    // Scale from reference coords to screen coords
    RECT screenRect;
    screenRect.left = static_cast<LONG>(refRect.left * m_scaleX);
    screenRect.top = static_cast<LONG>(refRect.top * m_scaleY);
    screenRect.right = static_cast<LONG>(refRect.right * m_scaleX);
    screenRect.bottom = static_cast<LONG>(refRect.bottom * m_scaleY);

    // Apply alpha to color
    XMVECTORF32 finalColor;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&finalColor), color);
    finalColor.f[3] = alpha;

    m_spriteBatch->Draw(m_whiteTexture.Get(), screenRect, finalColor);
}

void Renderer::DrawText(const std::wstring& text, XMFLOAT2 refPosition,
    FXMVECTOR color, float alpha, FontType font, float scale)
{
    SpriteFont* spriteFont = GetFont(font);
    if (!spriteFont) return;

    XMFLOAT2 screenPos;
    screenPos.x = refPosition.x * m_scaleX;
    screenPos.y = refPosition.y * m_scaleY;

    float finalScale = scale * std::min(m_scaleX, m_scaleY);

    XMVECTOR finalColor = XMVectorSetW(color, alpha);

    spriteFont->DrawString(
        m_spriteBatch.get(),
        text.c_str(),
        screenPos,
        finalColor,
        0.0f,
        XMFLOAT2(0.f, 0.f),
        finalScale
    );
}

void Renderer::DrawTextCentered(const std::wstring& text, XMFLOAT2 refPosition,
    FXMVECTOR color, float alpha, FontType font, float scale)
{
    SpriteFont* spriteFont = GetFont(font);
    if (!spriteFont) return;

    XMVECTOR textSize = spriteFont->MeasureString(text.c_str());
    XMFLOAT2 size;
    XMStoreFloat2(&size, textSize);

    float finalScale = scale * std::min(m_scaleX, m_scaleY);

    XMFLOAT2 origin(size.x * 0.5f, size.y * 0.5f);

    XMFLOAT2 screenPos;
    screenPos.x = refPosition.x * m_scaleX;
    screenPos.y = refPosition.y * m_scaleY;

    XMVECTOR finalColor = XMVectorSetW(color, alpha);

    spriteFont->DrawString(
        m_spriteBatch.get(),
        text.c_str(),
        screenPos,
        finalColor,
        0.0f,
        origin,
        finalScale
    );
}
