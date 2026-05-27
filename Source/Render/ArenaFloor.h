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

    com_ptr<ID3D11Buffer>             m_vertexBuffer;
    com_ptr<ID3D11Buffer>             m_indexBuffer;
    com_ptr<ID3D11Buffer>             m_constantBuffer;
    com_ptr<ID3D11VertexShader>       m_vertexShader;
    com_ptr<ID3D11PixelShader>        m_pixelShader;
    com_ptr<ID3D11InputLayout>        m_inputLayout;
    com_ptr<ID3D11BlendState>         m_blendState;
    com_ptr<ID3D11DepthStencilState>  m_depthStencilState;
    com_ptr<ID3D11RasterizerState>    m_rasterizerState;

    float m_time = 0.0f;
    float m_size = 500.0f;
    float m_speed = 0.8f;
    float m_brightness = 0.6f;
    float m_alpha = 0.8f;
};
