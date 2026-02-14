#include "pch.h"
#include "Projectile.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Projectile::Projectile()
    : m_isActive(false)
    , m_position(Vector3::Zero)
    , m_velocity(Vector3::Zero)
    , m_lifetime(0.0f)
    , m_maxLifetime(0.4f)      // Projectile lives for 1.5 seconds
    , m_speed(50.0f)           // Units per second
    , m_damage(5.0f)           // Damage on hit
    , m_radius(0.3f)           // Small orb
    , m_color(0.2f, 0.2f, 0.8f, 1.0f)
{
}

void Projectile::Initialize()
{
}

void Projectile::Spawn(
    const Vector3& position,
    const Vector3& direction,
    float speed,
    float damage)
{
    m_isActive = true;
    m_position = position;

    // Normalize direction and apply speed
    Vector3 dir = direction;
    dir.Normalize();
    m_velocity = dir * speed;

    m_speed = speed;
    m_damage = damage;
    m_lifetime = m_maxLifetime;  // Reset lifetime
}

void Projectile::Update(float deltaTime)
{
    if (!m_isActive) return;

    // Move based on velocity
    m_position += m_velocity * deltaTime;

    // Count down lifetime
    m_lifetime -= deltaTime;

    // Deactivate when expired
    if (m_lifetime <= 0.0f)
    {
        Deactivate();
    }
}

void Projectile::Render(const Matrix& view, const Matrix& projection, GeometricPrimitive* mesh)
{
    if (!m_isActive) return;
    if (!mesh) return;

    // Calculate rotation to face velocity direction
    Vector3 dir = m_velocity;
    dir.Normalize();

    // Create rotation from default (up) to velocity direction
    Vector3 up = Vector3::UnitY;  // Cylinder points up by default
    Quaternion rotation = Quaternion::FromToRotation(up, dir);

    Matrix world = Matrix::CreateFromQuaternion(rotation)
        * Matrix::CreateTranslation(m_position);

    mesh->Draw(world, view, projection, m_color);
}

void Projectile::Deactivate()
{
    m_isActive = false;
    m_position = Vector3::Zero;
    m_velocity = Vector3::Zero;
}