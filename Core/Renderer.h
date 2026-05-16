#pragma once

#include "DeviceResources.h"
#include "Source/Render/SceneRenderer.h"
#include <memory>
#include <string>
#include <SpriteBatch.h>
#include <SpriteFont.h>

enum class FontType { Title, Quote, Menu, Hud };

class RenderCommandQueue;
struct ImportedModelCommand;

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

    SceneRenderer* GetSceneRenderer() { return m_sceneRenderer.get(); }

    void BeginScene();
    void EndScene();

    void ExecuteRenderCommands(
        const RenderCommandQueue& queue,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition);

    void BeginViewmodelPass();

    void SetRenderResolution(int width, int height);

    int GetRenderWidth() const;
    int GetRenderHeight() const;

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
    void CreateImportedModelResources();
    void DrawImportedModelCommand(
        const ImportedModelCommand& command,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);

    // Device resources (not owned by this class)
    DX::DeviceResources* m_deviceResources;

    std::unique_ptr<SceneRenderer> m_sceneRenderer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_importedModelVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_importedModelPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_importedModelInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_importedModelConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_importedModelSolidRasterizer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_importedModelWireframeRasterizer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_importedModelSampler;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_importedModelAlphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_importedModelAdditiveBlendState;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_sceneRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneSRV;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneDepthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_sceneDSV;

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

    int m_renderWidth = 0;
    int m_renderHeight = 0;

    void CreateUIResources();
    void UpdateScaling();
};
