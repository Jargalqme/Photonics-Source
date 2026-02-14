#include "pch.h"
#include "Bullet.h"

Bullet::Bullet(DX::DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
	, m_boundingSphere(XMFLOAT3(0.0f,0.0f,0.0f), 0.25f)
	, m_position(Vector3::Zero)    // will be set when fired
	, m_direction(Vector3::Zero)   // will be set when fired
	, m_speed(20.0f)			   // meters per second - adjust for feel
	, m_active(false)			   // not flying yet
	, m_color(Colors::Red)

{
}

void Bullet::Initialize()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_mesh = GeometricPrimitive::CreateCube(context, 0.5f);
}

void Bullet::Update(float deltaTime)
{
	if (!m_active)
		return;

	// Move bullet
	m_position += m_direction * m_speed * deltaTime;

	m_boundingSphere.Center.x = m_position.x;
	m_boundingSphere.Center.y = m_position.y;
	m_boundingSphere.Center.z = m_position.z;

	// Deactivate if too far (cleanup)
	if (m_position.Length() > 100.0f)
		m_active = false;
}

void Bullet::Render(const Matrix& view, const Matrix& projection)
{
	if (!m_active) return;

	Matrix world = Matrix::CreateTranslation(m_position);
	m_mesh->Draw(world, view, projection, m_color);
}

void Bullet::Fire(const Vector3& from, const Vector3& target)
{
	m_position = from;				// Start at boss position

	m_boundingSphere.Center.x = m_position.x;
	m_boundingSphere.Center.y = m_position.y;
	m_boundingSphere.Center.z = m_position.z;


	// Calculate direction: target - from, then normalize
	m_direction = target - from;    // Vector pointing from boss to player
	m_direction.Normalize();		// Make length = 1 (just direction, no distance)

	m_active = true;				// Bullet is now flying
}

void Bullet::OnDeviceLost()
{
	m_mesh.reset();
}


