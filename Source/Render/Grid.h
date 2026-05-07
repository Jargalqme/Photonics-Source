#pragma once
#include "DeviceResources.h"

class Grid
{
public:
    Grid(DX::DeviceResources* deviceResources);
    ~Grid() = default;

    void initialize();

    void update();

    void render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);

    void renderPlane(const Matrix& world, const Matrix& view, const Matrix& projection);

    void finalize();

    void setLineColor(const DirectX::SimpleMath::Color& color) { m_lineColor = color; }
    void setBaseColor(const DirectX::SimpleMath::Color& color) { m_baseColor = color; }
    void setBeatPulse(float pulse) { m_beatPulse = pulse; }

    // デバッグUI用ポインタ
    float* getLineWidthXPtr() { return &m_lineWidthX; }
    float* getLineWidthYPtr() { return &m_lineWidthY; }
    float* getGridScalePtr() { return &m_gridScale; }
    float* getLineColorPtr() { return reinterpret_cast<float*>(&m_lineColor); }
    float* getBaseColorPtr() { return reinterpret_cast<float*>(&m_baseColor); }

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
    };

    struct ConstantBuffer
    {
        DirectX::SimpleMath::Matrix  worldViewProjection;
        DirectX::SimpleMath::Vector4 gridParams;   // x=lineWidthX, y=lineWidthY, z=scale
        DirectX::SimpleMath::Vector4 lineColor;
        DirectX::SimpleMath::Vector4 baseColor;
    };

    DX::DeviceResources* m_deviceResources;

    // === グリッドパラメータ ===
    static constexpr float GRID_SIZE     = 200.0f;
    static constexpr float FLOOR_Y       = -1.0f;
    static constexpr float WALL_HEIGHT   = 199.5f;
    static constexpr float WALL_DISTANCE = 200.0f;

    float m_gridSize = GRID_SIZE;
    float m_lineWidthX = 0.03f;
    float m_lineWidthY = 0.03f;
    float m_gridScale = 0.15f;
    float m_beatPulse = 0.0f;

    // エレクトリックブルー (#0066FF) + オービットネイビー (#0A1024)
    DirectX::SimpleMath::Color m_lineColor{ 0.0f, 0.4f, 1.0f, 1.0f };
    DirectX::SimpleMath::Color m_baseColor{ 0.039f, 0.063f, 0.141f, 1.0f };
    DirectX::SimpleMath::Color m_finalColor{ 0.0f, 0.0f, 0.0f, 1.0f };

    // === グリッド面のワールド行列 ===
    DirectX::SimpleMath::Matrix m_worldFloor = Matrix::Identity * Matrix::CreateTranslation(0, FLOOR_Y, 0);

    // === GPUリソース ===
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
