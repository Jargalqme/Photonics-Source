#pragma once
#include "DeviceResources.h"
#include "../ThirdParty/FastNoiseLite.h"
#include <vector>

class Terrain
{
public:
    Terrain(DX::DeviceResources* deviceResources);
    ~Terrain() = default;

    void initialize();
    void render(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection);
    void createDeviceDependentResources();
    void onDeviceLost();

    void rebuild();

    // Setters
    void setBeatPulse(float pulse) { m_beatPulse = pulse; }
    void setLineColor(const DirectX::SimpleMath::Color& color) { m_lineColor = color; }

    // ImGui getters
    float* getInnerRadiusPtr() { return &m_innerRadius; }
    float* getOuterRadiusPtr() { return &m_outerRadius; }
    float* getHeightScalePtr() { return &m_heightScale; }
    float* getBaseWidthPtr() { return &m_baseWidth; }
    float* getLineColorPtr() { return reinterpret_cast<float*>(&m_lineColor); }
    int* getSpikeCountPtr() { return &m_spikeCount; }

private:
    DX::DeviceResources* m_deviceResources;

    FastNoiseLite m_noise;

    // Params
    float m_innerRadius = 100.0f;
    float m_outerRadius = 250.0f;
    float m_heightScale = 90.0f;
    float m_baseWidth = 15.0f;
    int m_spikeCount = 48;
    float m_beatPulse = 0.0f;

    // Colors
    DirectX::SimpleMath::Color m_lineColor{ 0.7f, 0.0f, 1.0f, 1.0f };

    std::vector<DirectX::VertexPositionColor> m_vertices;

    // GPU Resources
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    std::unique_ptr<DirectX::BasicEffect> m_effect;

    void generateMesh();
    void uploadMesh();
};