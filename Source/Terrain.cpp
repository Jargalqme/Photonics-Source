#include "pch.h"
#include "Terrain.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Terrain::Terrain(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    m_noise.SetFrequency(0.1f);
}

void Terrain::Initialize()
{
    CreateDeviceDependentResources();
}

void Terrain::GenerateMesh()
{
    m_vertices.clear();

    for (int i = 0; i < m_spikeCount; i++)
    {
        float angle = (static_cast<float>(i) / m_spikeCount) * XM_2PI;
        float midRadius = (m_innerRadius + m_outerRadius) * 0.5f;

        // Spike center on the ring
        float cx = cosf(angle) * midRadius;
        float cz = sinf(angle) * midRadius;

        // Noise-based height
        float height = (m_noise.GetNoise(cx, cz) + 1.0f) * 0.5f;
        height *= m_heightScale;
        if (height < 5.0f) height = 5.0f;

        // Peak point
        Vector3 peak = Vector3(cx, height, cz);

        // Base corners — 4 points forming a diamond on the ground
        float half = m_baseWidth * 0.5f;
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        // Radial direction (outward from center) and tangent direction (along the ring)
        Vector3 radial = Vector3(cosA, 0, sinA);
        Vector3 tangent = Vector3(-sinA, 0, cosA);

        Vector3 b0 = Vector3(cx, 0, cz) + radial * half;   // outer
        Vector3 b1 = Vector3(cx, 0, cz) - radial * half;   // inner
        Vector3 b2 = Vector3(cx, 0, cz) + tangent * half;  // left
        Vector3 b3 = Vector3(cx, 0, cz) - tangent * half;  // right

        // Color
        XMFLOAT4 color = { m_lineColor.R(), m_lineColor.G(), m_lineColor.B(), m_lineColor.A() };

        // Dimmer base color
        XMFLOAT4 dimColor = { m_lineColor.R() * 0.3f, m_lineColor.G() * 0.3f, m_lineColor.B() * 0.3f, 1.0f };

        // Lines from each base corner to peak (4 edges going up)
        m_vertices.push_back({ XMFLOAT3(b0.x, b0.y, b0.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(peak.x, peak.y, peak.z), color });

        m_vertices.push_back({ XMFLOAT3(b1.x, b1.y, b1.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(peak.x, peak.y, peak.z), color });

        m_vertices.push_back({ XMFLOAT3(b2.x, b2.y, b2.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(peak.x, peak.y, peak.z), color });

        m_vertices.push_back({ XMFLOAT3(b3.x, b3.y, b3.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(peak.x, peak.y, peak.z), color });

        // Base diamond (4 edges connecting base corners)
        m_vertices.push_back({ XMFLOAT3(b0.x, b0.y, b0.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(b2.x, b2.y, b2.z), dimColor });

        m_vertices.push_back({ XMFLOAT3(b2.x, b2.y, b2.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(b1.x, b1.y, b1.z), dimColor });

        m_vertices.push_back({ XMFLOAT3(b1.x, b1.y, b1.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(b3.x, b3.y, b3.z), dimColor });

        m_vertices.push_back({ XMFLOAT3(b3.x, b3.y, b3.z), dimColor });
        m_vertices.push_back({ XMFLOAT3(b0.x, b0.y, b0.z), dimColor });
    }
}

void Terrain::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetLightingEnabled(false);

    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    DX::ThrowIfFailed(
        device->CreateInputLayout(
            inputElements,
            ARRAYSIZE(inputElements),
            shaderByteCode,
            byteCodeLength,
            m_inputLayout.ReleaseAndGetAddressOf()
        )
    );

    GenerateMesh();
    UploadMesh();
}

void Terrain::Rebuild()
{
    GenerateMesh();
    UploadMesh();
}

void Terrain::UploadMesh()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionColor) * m_vertices.size());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = m_vertices.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.ReleaseAndGetAddressOf())
    );
}

void Terrain::Render(const Matrix& view, const Matrix& projection)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_effect->SetWorld(Matrix::Identity);
    m_effect->SetView(view);
    m_effect->SetProjection(projection);
    m_effect->Apply(context);

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT stride = sizeof(VertexPositionColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    context->Draw(static_cast<UINT>(m_vertices.size()), 0);
}

void Terrain::OnDeviceLost()
{
    m_vertexBuffer.Reset();
    m_inputLayout.Reset();
    m_effect.reset();
}