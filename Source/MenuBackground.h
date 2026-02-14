#pragma once

#include "DeviceResources.h"
#include <memory>
#include <vector>

class MenuBackground
{
public:
    MenuBackground(DX::DeviceResources* deviceResources);
    ~MenuBackground() = default;

    MenuBackground(MenuBackground&&) = default;
    MenuBackground& operator=(MenuBackground&&) = default;

    MenuBackground(MenuBackground const&) = delete;
    MenuBackground& operator=(MenuBackground const&) = delete;

    void Initialize();
    void Update(float deltaTime);
    void Render();
    void OnDeviceLost();

    // Setters for customization
    void SetSpeed(float speed) { m_speed = speed; }
    void SetPatternScale(float scale) { m_patternScale = scale; }
    void SetWarpIntensity(float intensity) { m_warpIntensity = intensity; }
    void SetBrightness(float brightness) { m_brightness = brightness; }
    void SetChromaticOffset(float offset) { m_chromaticOffset = offset; }
    void SetColorTint(float r, float g, float b) { m_colorTint = { r, g, b }; }
    void SetVignetteStrength(float strength) { m_vignetteStrength = strength; }

    enum class ShaderType
    {
        Warping,
        Fractal,
        Wave
    };
    void SetShaderType(ShaderType type);

private:
    // Vertex Structure
    // Simple fullscreen quad vertex: position + UV
    struct MenuVertex
    {
        DirectX::XMFLOAT3 position;  // Clip space position (-1 to 1)
        DirectX::XMFLOAT2 uv;        // Texture coordinates (0 to 1)
    };

    // Constant Buffer
    struct MenuConstantBuffer
    {
        float Time;                          // 4 bytes
        DirectX::XMFLOAT2 Resolution;        // 8 bytes
        float Speed;                         // 4 bytes - Animation speed
        float PatternScale;                  // 4 bytes - Ring density
        float WarpIntensity;                 // 4 bytes - Distortion amount
        float Brightness;                    // 4 bytes - Line brightness
        float ChromaticOffset;               // 4 bytes - RGB split amount
        DirectX::XMFLOAT3 ColorTint;         // 12 bytes - RGB multiplier
        float VignetteStrength;            
    };  // Total: 48 bytes (aligned)

    // Device resources (not owned)
    DX::DeviceResources* m_deviceResources;

    // Time accumulator for animation
    float m_time;

    // Rendering Resources
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    // Vertex data
    std::vector<MenuVertex> m_vertices;
    size_t m_vertexCount;

    // Private methods
    void CreateFullscreenQuad();
    void CreateShaders();
    void CreateDeviceDependentResources();

    // Shader parameters (tweakable)
    float m_speed = 1.0f;              // Animation speed multiplier
    float m_patternScale = 9.0f;       // Ring density (default 9.0)
    float m_warpIntensity = 1.0f;      // Distortion strength
    float m_brightness = 0.01f;        // Line brightness
    float m_chromaticOffset = 0.07f;   // RGB separation
    float m_vignetteStrength = 0.0f;  // 0 = no vignette, 1 = full vignette
    DirectX::XMFLOAT3 m_colorTint = { 1.0f, 1.0f, 1.0f }; // RGB multiplier (white = no change)

    ShaderType m_shaderType = ShaderType::Fractal;
    // Pixel shaders
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShaderWarping;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShaderFractal;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShaderWave;

};