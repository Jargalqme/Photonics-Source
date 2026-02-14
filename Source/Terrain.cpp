#include "pch.h"
#include "Terrain.h"
#include "VertexTypes.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Terrain::Terrain(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_innerRadius(100.0f)    // Terrain starts here (arena edge)
    , m_outerRadius(250.0f)    // Terrain ends here
    , m_heightScale(90.0f)     // Max mountain height
    , m_segments(32)           // Points around the ring (more = smoother circle)
    , m_rings(3)               // Layers from inner to outer (more = smoother terrain)
    , m_color(0.7f, 0.0f, 1.0f, 1.0f) // prev: 1, 0, 0.85, 1
{
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
    m_noise.SetFrequency(0.1f);
}

void Terrain::Initialize()
{
    CreateDeviceDependentResources();
}

// Create vertices and indices
void Terrain::GenerateMesh()
{
    m_vertices.clear();
    m_indices.clear();

    // generate height data in a grid
    std::vector<std::vector<float>> heights(m_rings + 1, std::vector<float>(m_segments + 1));
    std::vector<std::vector<Vector3>> positions(m_rings + 1, std::vector<Vector3>(m_segments + 1));

    for (int ring = 0; ring <= m_rings; ring++)
    {
        float t = static_cast<float>(ring) / m_rings;
        float radius = m_innerRadius + t * (m_outerRadius - m_innerRadius);

        for (int seg = 0; seg <= m_segments; seg++)
        {
            float angle = (static_cast<float>(seg) / m_segments) * XM_2PI;
            float x = cosf(angle) * radius;
            float z = sinf(angle) * radius;

            float height = m_noise.GetNoise(x, z);
            height = (height + 1.0f) * 0.5f;
            float edgeFactor = t * t;
            height = height * m_heightScale * edgeFactor;
            if (ring == 0) height = 0.0f;

            heights[ring][seg] = height;
            positions[ring][seg] = Vector3(x, height, z);
        }
    }

    // create triangles with FLAT normals (each triangle has unique vertices)
    for (int ring = 0; ring < m_rings; ring++)
    {
        for (int seg = 0; seg < m_segments; seg++)
        {
            // Get the 4 corners of this quad
            Vector3 a = positions[ring][seg];
            Vector3 b = positions[ring][seg + 1];
            Vector3 c = positions[ring + 1][seg];
            Vector3 d = positions[ring + 1][seg + 1];

            // Triangle 1: A-C-B
            Vector3 normal1 = (c - a).Cross(b - a);
            normal1.Normalize();

            // Triangle 2: B-C-D
            Vector3 normal2 = (c - b).Cross(d - b);
            normal2.Normalize();

            // Height-based color
            auto makeColor = [this](float h) {
                float heightNorm = (h / m_heightScale) * 0.3f + 0.7f;
                return XMFLOAT4(
                    m_color.R() * heightNorm,
                    m_color.G() * heightNorm,
                    m_color.B() * heightNorm,
                    1.0f
                );
                };

            // Triangle 1 vertices (each has SAME normal = flat face)
            uint32_t baseIndex = static_cast<uint32_t>(m_vertices.size());

            VertexPositionNormalColor v;

            v.position = XMFLOAT3(a.x, a.y, a.z);
            v.normal = XMFLOAT3(normal1.x, normal1.y, normal1.z);
            v.color = makeColor(a.y);
            m_vertices.push_back(v);

            v.position = XMFLOAT3(c.x, c.y, c.z);
            v.normal = XMFLOAT3(normal1.x, normal1.y, normal1.z);
            v.color = makeColor(c.y);
            m_vertices.push_back(v);

            v.position = XMFLOAT3(b.x, b.y, b.z);
            v.normal = XMFLOAT3(normal1.x, normal1.y, normal1.z);
            v.color = makeColor(b.y);
            m_vertices.push_back(v);

            m_indices.push_back(baseIndex);
            m_indices.push_back(baseIndex + 1);
            m_indices.push_back(baseIndex + 2);

            // Triangle 2 vertices
            baseIndex = static_cast<uint32_t>(m_vertices.size());

            v.position = XMFLOAT3(b.x, b.y, b.z);
            v.normal = XMFLOAT3(normal2.x, normal2.y, normal2.z);
            v.color = makeColor(b.y);
            m_vertices.push_back(v);

            v.position = XMFLOAT3(c.x, c.y, c.z);
            v.normal = XMFLOAT3(normal2.x, normal2.y, normal2.z);
            v.color = makeColor(c.y);
            m_vertices.push_back(v);

            v.position = XMFLOAT3(d.x, d.y, d.z);
            v.normal = XMFLOAT3(normal2.x, normal2.y, normal2.z);
            v.color = makeColor(d.y);
            m_vertices.push_back(v);

            m_indices.push_back(baseIndex);
            m_indices.push_back(baseIndex + 1);
            m_indices.push_back(baseIndex + 2);
        }
    }
}

void Terrain::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Create BasicEffect for rendering
    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);   // Use vertex colors
    m_effect->SetLightingEnabled(true);      // Enable lighting
    m_effect->EnableDefaultLighting();       // Use default 3-light setup

    // Get shader bytecode from effect
    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

    // Define input layout matching TerrainVertex struct
    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

    // Generate the mesh data
    GenerateMesh();

    // Create vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionNormalColor) * m_vertices.size());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = m_vertices.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.ReleaseAndGetAddressOf())
    );

    // Create index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * m_indices.size());
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = m_indices.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.ReleaseAndGetAddressOf())
    );
}

// Regenerate mesh after parameter change
void Terrain::Rebuild()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Regenerate mesh with new parameters
    GenerateMesh();

    // Recreate vertex buffer
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionNormalColor) * m_vertices.size());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = m_vertices.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.ReleaseAndGetAddressOf())
    );

    // Recreate index buffer
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * m_indices.size());
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = m_indices.data();

    DX::ThrowIfFailed(
        device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.ReleaseAndGetAddressOf())
    );
}

void Terrain::Render(const Matrix& view, const Matrix& projection)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Set effect matrices
    m_effect->SetWorld(Matrix::Identity);
    m_effect->SetView(view);
    m_effect->SetProjection(projection);
    m_effect->Apply(context);

    // Set input layout and topology
    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set vertex buffer
    UINT stride = sizeof(VertexPositionNormalColor);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // Set index buffer
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // Draw
    context->DrawIndexed(static_cast<UINT>(m_indices.size()), 0, 0);
}

// Clean up GPU resources
void Terrain::OnDeviceLost()
{
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_inputLayout.Reset();
    m_effect.reset();
}