#pragma once

#include "DeviceResources.h"
#include "Render/Pipeline/SceneRenderer.h"
#include "Render/Lighting/SceneLighting.h"
#include <memory>
#include <string>
#include <SpriteBatch.h>
#include <SpriteFont.h>

enum class FontType { Title, Quote, Menu, Hud };

class RenderCommandQueue;
struct ImportedModelCommand;

class Renderer final : noncopyable
{
public:
    Renderer(DX::DeviceResources* deviceResources);
    ~Renderer() = default;

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void OnDeviceLost();

    // Helper to get device resources for other systems
    [[nodiscard]] DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }

    [[nodiscard]] SceneRenderer* GetSceneRenderer() { return m_sceneRenderer.get(); }

    void BeginScene();
    void EndScene();

    void ExecuteRenderCommands(
        const RenderCommandQueue& queue,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition,
        const SceneLighting& lighting);

    void BeginViewmodelPass();

    void SetRenderResolution(int width, int height);

    [[nodiscard]] int GetRenderWidth() const;
    [[nodiscard]] int GetRenderHeight() const;

    void ApplyPostProcess();

    [[nodiscard]] ID3D11RenderTargetView* GetSceneRTV() { return m_sceneRTV.Get(); }

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
    [[nodiscard]] RECT GetFullscreenRect() const { return RECT{ 0, 0, static_cast<LONG>(REF_WIDTH), static_cast<LONG>(REF_HEIGHT) }; }

    // Resolution scaling info
    [[nodiscard]] float GetScaleX() const { return m_scaleX; }
    [[nodiscard]] float GetScaleY() const { return m_scaleY; }
    [[nodiscard]] DirectX::XMFLOAT2 GetScreenSize() const { return DirectX::XMFLOAT2(m_screenWidth, m_screenHeight); }

    // Reference resolution (design UI for this)
    static constexpr float REF_WIDTH = 1920.0f;
    static constexpr float REF_HEIGHT = 1080.0f;

    void SetIrradianceSRV(ID3D11ShaderResourceView* srv) { m_irradianceSRV = srv; }

private:
    DirectX::SpriteFont* GetFont(FontType type);
    void CreateImportedModelResources();
    void DrawImportedModelCommand(
        const ImportedModelCommand& command,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition,
        const SceneLighting& lighting);

    // Device resources (not owned by this class)
    DX::DeviceResources* m_deviceResources;

    std::unique_ptr<SceneRenderer> m_sceneRenderer;

    com_ptr<ID3D11VertexShader>    m_importedModelVS;
    com_ptr<ID3D11PixelShader>     m_importedModelPS;
    com_ptr<ID3D11InputLayout>     m_importedModelInputLayout;
    com_ptr<ID3D11Buffer>          m_importedModelConstantBuffer;
    com_ptr<ID3D11RasterizerState> m_importedModelSolidRasterizer;
    com_ptr<ID3D11RasterizerState> m_importedModelWireframeRasterizer;
    com_ptr<ID3D11SamplerState>    m_importedModelSampler;
    com_ptr<ID3D11BlendState>      m_importedModelAlphaBlendState;
    com_ptr<ID3D11BlendState>      m_importedModelAdditiveBlendState;

    ID3D11ShaderResourceView* m_irradianceSRV = nullptr;

    com_ptr<ID3D11Texture2D>          m_sceneTexture;
    com_ptr<ID3D11RenderTargetView>   m_sceneRTV;
    com_ptr<ID3D11ShaderResourceView> m_sceneSRV;

    com_ptr<ID3D11Texture2D>        m_sceneDepthTexture;
    com_ptr<ID3D11DepthStencilView> m_sceneDSV;

    // 2D / UI Resources
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::SpriteFont>  m_fontTitle;  // 64pt Bold
    std::unique_ptr<DirectX::SpriteFont>  m_fontQuote;  // 42pt Regular
    std::unique_ptr<DirectX::SpriteFont>  m_fontMenu;   // 32pt Regular
    std::unique_ptr<DirectX::SpriteFont>  m_fontHud;    // 24pt Regular
    com_ptr<ID3D11ShaderResourceView>     m_whiteTexture;  // 1x1 white pixel for DrawRect

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
