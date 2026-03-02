#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include "Projectile.h"
#include <vector>

class ProjectilePool
{
public:
    ProjectilePool(DX::DeviceResources* deviceResources);
    ~ProjectilePool() = default;

    void initialize(size_t poolSize = 100);
    void update(float deltaTime);
    void render(const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection);

    // Fire a projectile - returns true if one was available
    bool spawn(const DirectX::SimpleMath::Vector3& position,
        const DirectX::SimpleMath::Vector3& direction,
        float speed = 50.0f,
        float damage = 10.0f);

    // For collision detection
    std::vector<Projectile>& GetProjectiles() { return m_projectiles; }

    // Stats for debugging
    int GetActiveCount() const;
    int GetPoolSize() const { return static_cast<int>(m_projectiles.size()); }
    void onDeviceLost();

private:
    DX::DeviceResources* m_deviceResources;
    std::vector<Projectile> m_projectiles;
    std::unique_ptr<DirectX::GeometricPrimitive> m_sharedMesh;
};