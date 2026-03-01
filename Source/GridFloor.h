#pragma once
#include "DeviceResources.h"

class GridFloor
{
public:
    GridFloor(DX::DeviceResources* deviceResources);
    ~GridFloor() = default;

    void initialize();
    void update();
    void render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);
    void renderPlane(const Matrix& world, const Matrix& view, const Matrix& projection);
    void finalize();

    // セッター
    void setLineColor(const DirectX::SimpleMath::Color& color) { m_lineColor = color; }
    void setBaseColor(const DirectX::SimpleMath::Color& color) { m_baseColor = color; }
    void setBeatPulse(float pulse) { m_beatPulse = pulse; }

    // ImGui ゲッター
    float* getLineWidthXPtr() { return &m_lineWidthX; }
    float* getLineWidthYPtr() { return &m_lineWidthY; }
    float* getGridScalePtr() { return &m_gridScale; }
    float* getLineColorPtr() { return reinterpret_cast<float*>(&m_lineColor); }
    float* getBaseColorPtr() { return reinterpret_cast<float*>(&m_baseColor); }

private:
    // 頂点
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
    };

    // コンスタントバッファ
    struct ConstantBuffer
    {
        DirectX::SimpleMath::Matrix  worldViewProjection;
        DirectX::SimpleMath::Vector4 gridParams;   // x=lineWidthX, y=lineWidthY, z=scale
        DirectX::SimpleMath::Vector4 lineColor;
        DirectX::SimpleMath::Vector4 baseColor;
    };

    // Device (not owned)
    DX::DeviceResources* m_deviceResources;

    // Grid parameters
    float m_gridSize = 200.0f;
    float m_lineWidthX = 0.035f;
    float m_lineWidthY = 0.035f;
    float m_gridScale = 0.15f;
    float m_beatPulse = 0.0f; // 0 = on beat, 0.5 = mid-beat, 1.0 = next beat

    // Colors
    DirectX::SimpleMath::Color m_lineColor{ 1.0f, 0.0f, 1.0f, 1.0f };
    DirectX::SimpleMath::Color m_baseColor{ 0.25f, 0.30f, 0.4f, 1.0f };
    DirectX::SimpleMath::Color m_finalColor{ 0.0f, 0.0f, 0.0f, 0.0f };

    // Gridfloor
    DirectX::SimpleMath::Matrix m_worldFloor = Matrix::Identity * Matrix::CreateTranslation(0, -1.0f, 0);
    DirectX::SimpleMath::Matrix m_worldBack = Matrix::Identity * Matrix::CreateRotationX(-XM_PIDIV2) * Matrix::CreateTranslation(0, 199.5f, -200.0f);
    DirectX::SimpleMath::Matrix m_worldFront = Matrix::Identity * Matrix::CreateRotationX(XM_PIDIV2) * Matrix::CreateTranslation(0, 199.5f, 200.0f);
    DirectX::SimpleMath::Matrix m_worldLeft = Matrix::Identity * Matrix::CreateRotationZ(XM_PIDIV2) * Matrix::CreateTranslation(-200.0f, 199.5f, 0);
    DirectX::SimpleMath::Matrix m_worldRight = Matrix::Identity * Matrix::CreateRotationZ(-XM_PIDIV2) * Matrix::CreateTranslation(200.0f, 199.5f, 0);

    // GPU Resources
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};


