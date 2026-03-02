#pragma once

#include "pch.h"
#include "DeviceResources.h"
#include <vector>

class DeathBeamPool
{
public:
    DeathBeamPool(DX::DeviceResources* deviceResources);
    ~DeathBeamPool() = default;

    void initialize(size_t poolSize = 20);
    void update(float deltaTime);
    void render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition);
    void onDeviceLost();

    // Trigger a death beam at position
    void trigger(const DirectX::SimpleMath::Vector3& position);

    // Customization
    void setColor(const DirectX::SimpleMath::Color& color) { m_color = color; }
    void setHeight(float height) { m_beamHeight = height; }
    void setWidth(float width) { m_beamWidth = width; }
    void setDuration(float duration) { m_duration = duration; }

private:
    struct DeathBeam // uninit member variable warning!
    {
        DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
        float life = 0.0f;
        bool active = false;
    };

    struct BeamVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 uv;
    };

    struct BeamConstantBuffer
    {
        DirectX::SimpleMath::Matrix viewProjection;
        DirectX::SimpleMath::Vector3 beamStart;
        float beamWidth;
        DirectX::SimpleMath::Vector3 beamEnd;
        float beamLife;
        DirectX::SimpleMath::Vector4 beamColor;
        DirectX::SimpleMath::Vector3 cameraPosition;
        float time;
    };

    void createDeviceDependentResources();
    void createShaders();
    void createGeometry();

    DX::DeviceResources* m_deviceResources;
    std::vector<DeathBeam> m_beams;

    // Parameters
    DirectX::SimpleMath::Color m_color;
    float m_beamHeight;
    float m_beamWidth;
    float m_duration;
    float m_time;

    // GPU resources
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};