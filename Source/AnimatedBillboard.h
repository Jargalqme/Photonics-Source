#pragma once
#include "DeviceResources.h"

class AnimatedBillboard
{
public:
    AnimatedBillboard(DX::DeviceResources* deviceResources);
    ~AnimatedBillboard() = default;

    void initialize();
    void update(float deltaTime);
    void render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition);
    void finalize();

    void setPosition(DirectX::SimpleMath::Vector3 position) { m_position = position; }
    void setSize(float size) { m_size = size; }
    void setFrameRate(float fps) { m_framesPerSecond = fps; }

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 uv;
    };

    struct __declspec(align(16)) ConstantBuffer
    {
        DirectX::SimpleMath::Matrix viewProjection;
        DirectX::SimpleMath::Vector3 worldPosition;
        float billboardSize;
        DirectX::SimpleMath::Vector3 cameraRight;
        float frameOffsetU;
        DirectX::SimpleMath::Vector3 cameraUp;
        float frameOffsetV;
        float frameSizeU;
        float frameSizeV;
        float padding[2];
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
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_samplerState;

    // Animation state
    int   m_currentFrame = 0;
    float m_frameTimer = 0.0f;
    float m_framesPerSecond = 12.0f;

    // Cached UV offsets (computed in update)
    float m_frameOffsetU = 0.0f;
    float m_frameOffsetV = 0.0f;

    // Config
    DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
    float m_size = 1.0f;
};