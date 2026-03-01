#include "pch.h"
#include "Renderer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Renderer::Renderer(DX::DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
{
	m_colorMaskEffect = std::make_unique<ColorMaskEffect>(deviceResources);
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
	m_colorMaskEffect->CreateDeviceDependentResources();
	CreateUIResources();
}

void Renderer::CreateWindowSizeDependentResources()
{
    m_colorMaskEffect->CreateWindowSizeDependentResources();

    // Create intermediate render target for scene
    auto device = m_deviceResources->GetD3DDevice();
    auto size = m_deviceResources->GetOutputSize();
    UINT width = static_cast<UINT>(size.right - size.left);
    UINT height = static_cast<UINT>(size.bottom - size.top);

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr,
        m_sceneTexture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateRenderTargetView(m_sceneTexture.Get(), nullptr,
        m_sceneRTV.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(device->CreateShaderResourceView(m_sceneTexture.Get(), nullptr,
        m_sceneSRV.ReleaseAndGetAddressOf()));

    // Update UI scaling
    m_screenWidth = static_cast<float>(width);
    m_screenHeight = static_cast<float>(height);
    UpdateScaling();
}

void Renderer::ApplyPostProcess()
{
    auto backbuffer = m_deviceResources->GetRenderTargetView();
    m_colorMaskEffect->Process(m_sceneSRV.Get(), backbuffer);
}

void Renderer::OnDeviceLost()
{
    m_colorMaskEffect->OnDeviceLost();
    m_sceneTexture.Reset();
    m_sceneRTV.Reset();
    m_sceneSRV.Reset();

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
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    // Scene color
    static const float clearColor[4] = { 0.02f, 0.08f, 0.12f, 1.0f };

    // Get RTV as local variable (important)
    ID3D11RenderTargetView* rtv = m_sceneRTV.Get();

    // Set render target
    context->OMSetRenderTargets(1, &rtv, depthStencil);

    // Clear
    context->ClearRenderTargetView(rtv, clearColor);
    context->ClearDepthStencilView(depthStencil,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set viewport
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
}

void Renderer::EndScene()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Unbind scene texture so we can read from it
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, nullptr);

    // Copy to backbuffer via ColorMask
    ApplyPostProcess();
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
