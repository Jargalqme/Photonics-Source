#pragma once
#include "DeviceResources.h"

class BeamWeapon
{
public:
    BeamWeapon(DX::DeviceResources* deviceResources);
    ~BeamWeapon() = default;

    void initialize();
    void update(float deltaTime);
    void render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition);
    void finalize();

    // Setters
    void setColor(const DirectX::SimpleMath::Color& color) { m_color = color; }
    void setWidth(float width) { m_width = width; }
    void setMaxRange(float range) { m_maxRange = range; }
    void setDuration(float duration) { m_maxLife = duration; }
    void setCooldown(float cooldown) { m_cooldown = cooldown; }
    void setCollisionDuration(float duration) { m_collisionDuration = duration; }

    // Getters
    bool isActive() const { return m_isActive; }
    bool canFire() const { return m_cooldownTimer <= 0.0f; }
    bool isCollisionActive() const { return m_collisionLife > 0.0f; }
    float getCooldownProgress() const;
    Vector3 getStart() const { return m_start; }
    Vector3 getEnd() const { return m_end; }
    Vector3 getDirection() const { Vector3 dir = m_end - m_start; dir.Normalize(); return dir; }
    float getMaxRange() const { return m_maxRange; }

    // Getters for ImGui
    float* getWidthPtr() { return &m_width; }
    float* getMaxRangePtr() { return &m_maxRange; }
    DirectX::SimpleMath::Color* getColorPtr() { return &m_color; }

    // Actions
    void fire(const DirectX::SimpleMath::Vector3& startPosition, const DirectX::SimpleMath::Vector3& direction);

private:

    // Constant buffer
    struct ConstantBuffer
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

    // Device (not owned)
    DX::DeviceResources* m_deviceResources;

    // Beam parameters
    float m_width = 1.5f;
    float m_maxRange = 150.0f;
    float m_maxLife = 0.5f;
    float m_cooldown = 0.5f;

    // Beam runtime state
    bool m_isActive = false;
    DirectX::SimpleMath::Vector3 m_start = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 m_end = DirectX::SimpleMath::Vector3::Zero;
    float m_life = 0.0f;
    float m_time = 0.0f;
    float m_cooldownTimer = 0.0f;
    float m_collisionLife = 0.0f;
    float m_collisionDuration = 0.1f;

    // Color
    DirectX::SimpleMath::Color m_color{ 1.0f, 0.0f, 0.0f, 1.0f };

    // GPU Resources
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};
