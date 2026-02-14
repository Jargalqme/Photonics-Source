#pragma once
#include "pch.h"
#include "DeviceResources.h"

class Projectile
{
public:
    Projectile();
    ~Projectile() = default;

    // Core methods
    void Initialize();
    void Spawn(const DirectX::SimpleMath::Vector3& position,
        const DirectX::SimpleMath::Vector3& direction,
        float speed,
        float damage);
    void Update(float deltaTime);
    void Render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        DirectX::GeometricPrimitive* mesh);
    void Deactivate();

    // State
    bool IsActive() const { return m_isActive; }

    // For collision
    DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
    float GetRadius() const { return m_radius; }
    float GetDamage() const { return m_damage; }
private:
    bool m_isActive;

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_velocity;

    float m_lifetime;      // Time remaining
    float m_maxLifetime;   // Total lifespan
    float m_speed;
    float m_damage;
    float m_radius;        // For collision & size

    DirectX::SimpleMath::Color m_color;
};
