#pragma once

#include "DeviceResources.h"
#include "Source/ColorMaskEffect.h"
#include <memory>
#include <string>
#include <SpriteBatch.h>
#include <SpriteFont.h>

enum class FontType { Title, Quote, Menu, Hud };

class Renderer final
{
public:
    Renderer(DX::DeviceResources* deviceResources);
    ~Renderer() = default;

    Renderer(Renderer&&) = default;
    Renderer& operator=(Renderer&&) = default;

    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void OnDeviceLost();

    // Helper to get device resources for other systems
    DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }

    ColorMaskEffect* GetColorMaskEffect() { return m_colorMaskEffect.get(); }

    void BeginScene();
    void EndScene();

    void ApplyPostProcess();

    ID3D11RenderTargetView* GetSceneRTV() { return m_sceneRTV.Get(); }

    // 2D / UI Rendering
    // Call BeginUI before drawing 2D elements, EndUI when done
    void BeginUI();
    void EndUI();

    // Draw filled rectangle (use for fade overlay, backgrounds)
    // Position/size in reference coordinates (1920x1080)
    void DrawRect(const RECT& rect, DirectX::FXMVECTOR color, float alpha = 1.0f);

    // Draw text
    void DrawText(const std::wstring& text, DirectX::XMFLOAT2 position,
        DirectX::FXMVECTOR color, float alpha = 1.0f,
        FontType font = FontType::Menu, float scale = 1.0f);

    // Centered text helper
    void DrawTextCentered(const std::wstring& text, DirectX::XMFLOAT2 position,
        DirectX::FXMVECTOR color, float alpha = 1.0f,
        FontType font = FontType::Menu, float scale = 1.0f);

    // Get fullscreen rect in reference coordinates
    RECT GetFullscreenRect() const {
        return RECT{ 0, 0,
static_cast<LONG>(REF_WIDTH), static_cast<LONG>(REF_HEIGHT) };
    }

    // Resolution scaling info
    float GetScaleX() const { return m_scaleX; }
    float GetScaleY() const { return m_scaleY; }
    DirectX::XMFLOAT2 GetScreenSize() const { return DirectX::XMFLOAT2(m_screenWidth, m_screenHeight); }

    // Reference resolution (design UI for this)
    static constexpr float REF_WIDTH = 1920.0f;
    static constexpr float REF_HEIGHT = 1080.0f;



private:
    DirectX::SpriteFont* GetFont(FontType type);
    // Device resources (not owned by this class)
    DX::DeviceResources* m_deviceResources;

    std::unique_ptr<ColorMaskEffect> m_colorMaskEffect;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_sceneRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneSRV;

    // 2D / UI Resources
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont> m_fontTitle;  // 64pt Bold
    std::unique_ptr<DirectX::SpriteFont> m_fontQuote;  // 42pt Regular
    std::unique_ptr<DirectX::SpriteFont> m_fontMenu;   // 32pt Regular
    std::unique_ptr<DirectX::SpriteFont> m_fontHud;    // 24pt Regular
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_whiteTexture;  // 1x1 white pixel for DrawRect

    // Resolution scaling
    float m_screenWidth = REF_WIDTH;
    float m_screenHeight = REF_HEIGHT;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;

    void CreateUIResources();
    void UpdateScaling();
};