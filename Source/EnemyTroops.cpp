#include "pch.h"
#include "EnemyTroops.h"

Enemy::Enemy(DX::DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
	, m_type(EnemyType::Troop_Type_1)
	, m_position(Vector3::Zero)
	, m_targetPosition(Vector3::Zero)
	, m_boundingSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f)
	, m_speed(8.0f)
	, m_health(10.0f)
	, m_active(false)
	, m_color(Colors::Gray)
	, m_useWaveMovement(false)
	, m_waveAmplitude(3.0f)
	, m_waveFrequency(2.0f)
	, m_wavePhase(0.0f)
	, m_elapsedTime(0.0f)
	, m_baseSpeed(8.0f)
{
}


void Enemy::initialize()
{
}


void Enemy::update(float deltaTime)
{
	if (!m_active)
		return;

	m_elapsedTime += deltaTime;

	// Direction to target
	Vector3 toTarget = m_targetPosition - m_position;
	float distance = toTarget.Length();

	if (distance > 1.0f)  // Not at target yet
	{
		toTarget.Normalize();

		// Base movement toward target
		Vector3 movement = toTarget * m_speed * deltaTime;

		// Apply wave movement (perpendicular oscillation)
		if (m_useWaveMovement)
		{
			// Perpendicular vector (rotate 90 degrees on XZ plane)
			Vector3 perpendicular(-toTarget.z, 0.0f, toTarget.x);

			// Sin wave offset
			float waveOffset = sinf(m_elapsedTime * m_waveFrequency + m_wavePhase) * m_waveAmplitude;

			// Apply perpendicular offset (scaled by deltaTime for smooth motion)
			float prevWave = sinf((m_elapsedTime - deltaTime) * m_waveFrequency + m_wavePhase) * m_waveAmplitude;
			float waveDelta = waveOffset - prevWave;

			movement += perpendicular * waveDelta;
		}

		m_position += movement;
	}

	// Update bounding sphere
	m_boundingSphere.Center.x = m_position.x;
	m_boundingSphere.Center.y = m_position.y;
	m_boundingSphere.Center.z = m_position.z;

	// Hit flash decay
	if (m_hitFlashTimer > 0.0f)
	{
		m_hitFlashTimer -= deltaTime;
		m_color = Color::Lerp(m_originalColor, m_hitColor, m_hitFlashTimer / 0.1f);
	}
	else
	{
		m_color = m_originalColor;	
	}
}

void Enemy::render(const Matrix& view, const Matrix& projection)
{

	if (!m_active)
		return;
	Matrix world = Matrix::CreateTranslation(m_position);
	m_mesh->Draw(world, view, projection, m_color);
}

void Enemy::spawn(EnemyType type, const Vector3& startPos, const Vector3& targetPos)
{
	m_type = type;
	m_position = startPos;
	m_targetPosition = targetPos;
	m_active = true;

	// Reset wave state
	m_elapsedTime = 0.0f;
	m_wavePhase = static_cast<float>(rand()) / RAND_MAX * 6.28f;  // Random 0 to 2*PI
	m_useWaveMovement = false;  // Will be set by GameScene based on phase

	m_boundingSphere.Center.x = startPos.x;
	m_boundingSphere.Center.y = startPos.y;
	m_boundingSphere.Center.z = startPos.z;

	auto context = m_deviceResources->GetD3DDeviceContext();

	m_speed = 8.0f;
	m_baseSpeed = 8.0f;
	m_color = Color(0.15f, 0.15f, 0.15f);
	m_originalColor = m_color;
	m_mesh = GeometricPrimitive::CreateDodecahedron(context, 1.5f);
	m_boundingSphere.Radius = 1.5f;
}

void Enemy::takeDamage(float amount)
{
	m_health -= amount;

	// Hit feedback - flash white
	m_hitFlashTimer = 0.1f; // Flash for 0.1 seconds

	if (m_health <= 0.0f)
	{
		m_active = false;
	}
}

void Enemy::setWaveMovement(bool enabled, float amplitude, float frequency)
{
	m_useWaveMovement = enabled;
	m_waveAmplitude = amplitude;
	m_waveFrequency = frequency;
}

void Enemy::deactivate()
{
	m_active = false;
}

void Enemy::onDeviceLost()
{
	m_mesh.reset();
}


