#include "pch.h"
#include "Core.h"

Core::Core(DX::DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
	, m_boundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 1.5f)
	, m_color(Colors::AntiqueWhite)
	, m_health(100.0f)
	, m_maxHealth(100.0f)
	, m_alive(true)
{
}

void Core::Initialize()
{
	BuildCore();
}

void Core::Update(float deltaTime)
{
}

void Core::Render(const Matrix& view, const Matrix& projection)
{
	if (!m_alive) return;
    Matrix coreWorld = m_transform.GetMatrix();

	for (const auto& part : m_parts)
	{
		Matrix local = Matrix::CreateRotationX(part.localRotation.x)
			         * Matrix::CreateRotationY(part.localRotation.y)
			         * Matrix::CreateRotationZ(part.localRotation.z)
			         * Matrix::CreateTranslation(part.localPosition);

		Matrix world = local * coreWorld;
		part.mesh->Draw(world, view, projection, part.color);
	}
}

void Core::TakeDamage(float amount)
{
	m_health -= amount;
	if (m_health <= 0)
	{
		m_alive = false;
	}
}

void Core::SetPosition(const Vector3& pos)
{
	m_transform.position = pos;
	m_boundingSphere.Center.x = pos.x;
	m_boundingSphere.Center.y = pos.y;
	m_boundingSphere.Center.z = pos.z;
}

void Core::SetColor(const Color& color)
{
    m_color = color;

    // Rebuild immediately if already initialized
    if (!m_parts.empty())
    {
        BuildCore();
    }
}

void Core::Reset(float health)
{
    m_health = health;
    m_maxHealth = health;
    m_alive = true;
}

void Core::BuildCore()
{
    auto ctx = m_deviceResources->GetD3DDeviceContext();
    m_parts.clear();

    // Glow version of the core color
    Color glowColor = m_color;

    // === MAIN CRYSTAL (Icosahedron) ===
    m_parts.push_back(MeshPart::CreateIcosahedron(ctx,
        0.6f,  // size
        Vector3::Zero,
        m_color));

    // === ORBITING SHARDS (3 Dodecahedrons) ===
    for (int i = 0; i < 3; i++)
    {
        float angle = (i / 3.0f) * XM_2PI;
        Vector3 shardPos(
            cosf(angle) * 1.3f,
            sinf(angle * 2) * 0.4f,
            sinf(angle) * 1.3f
        );

        m_parts.push_back(MeshPart::CreateDodecahedron(ctx,
            0.36f,
            shardPos,
            m_color));
    }

    // === HORIZONTAL RING ===
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        2.2f, 0.05f,
        Vector3::Zero,
        Vector3(XM_PIDIV2, 0, 0),
        m_color));

    // === 縦　リング ===
    m_parts.push_back(MeshPart::CreateTorus(ctx,
        2.0f,                       //　直径
        0.04f,                      //　厚さ
        Vector3::Zero,              //　位置
        Vector3(0, 0, XM_PIDIV2),   //　回転
        m_color));                  //　色

    // Update bounding sphere
    m_boundingSphere.Radius = 1.5f;
}

void Core::OnDeviceLost()
{
	m_parts.clear();
}