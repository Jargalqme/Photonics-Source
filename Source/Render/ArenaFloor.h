#pragma once

#include "DeviceResources.h"

class ArenaFloor
{
public:
    ArenaFloor(DX::DeviceResources* deviceResources);
    ~ArenaFloor() = default;

    void initialize();
    void finalize();
    void update(float deltaTime);
    void render(const DirectX::SimpleMath::Matrix& view,
                const DirectX::SimpleMath::Matrix& projection);

    void setSize(float size) { m_size = size; }
    void setSpeed(float speed) { m_speed = speed; }
    void setBrightness(float b) { m_brightness = b; }
    void setAlpha(float a) { m_alpha = a; }

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
    };

    struct ConstantBuffer
    {
        DirectX::XMFLOAT4X4 worldViewProjection;
        float time;
        float speed;
        float brightness;
        float alpha;
    };

    DX::DeviceResources* m_deviceResources;

    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>             m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>       m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>        m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11BlendState>         m_blendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>  m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>    m_rasterizerState;

    float m_time = 0.0f;
    float m_size = 200.0f;
    float m_speed = 0.5f;
    float m_brightness = 0.06f;
    float m_alpha = 0.8f;
};
