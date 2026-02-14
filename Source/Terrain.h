#pragma once
#include "DeviceResources.h"
#include "FastNoiseLite.h"
#include <vector>

class Terrain
{
public:
    Terrain(DX::DeviceResources* deviceResources);
    ~Terrain() = default;

    void Initialize();
    void Render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);
    void CreateDeviceDependentResources();
    void OnDeviceLost();

    void Rebuild();

    // imgui
    float* GetInnerRadiusPtr() { return &m_innerRadius; }
    float* GetOuterRadiusPtr() { return &m_outerRadius; }
    float* GetHeightScalePtr() { return &m_heightScale; }
    float* GetColorPtr() { return reinterpret_cast<float*>(&m_color); }
    int* GetSegmentsPtr() { return &m_segments; }
    int* GetRingsPtr() { return &m_rings; }

private:
    DX::DeviceResources* m_deviceResources;

    FastNoiseLite m_noise;

    float m_innerRadius;
    float m_outerRadius;
    float m_heightScale;

    int m_segments;
    int m_rings;
    DirectX::SimpleMath::Color m_color;

    std::vector<DirectX::VertexPositionNormalColor> m_vertices;
    std::vector<uint32_t> m_indices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    std::unique_ptr<DirectX::BasicEffect> m_effect;

    void GenerateMesh();
};

