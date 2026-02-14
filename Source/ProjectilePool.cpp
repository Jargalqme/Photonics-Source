#include "pch.h"
#include "ProjectilePool.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

ProjectilePool::ProjectilePool(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}


void ProjectilePool::Initialize(size_t poolSize)
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Create ONE shared mesh for all projectiles
    m_sharedMesh = GeometricPrimitive::CreateSphere(context, 0.3f);

    // Reserve projectiles
    m_projectiles.resize(poolSize);

    for (auto& projectile : m_projectiles)
    {
        projectile.Initialize();
    }
}

bool ProjectilePool::Spawn(
    const Vector3& position,
    const Vector3& direction,
    float speed,
    float damage)
{
    // Find first inactive projectile
    for (auto& projectile : m_projectiles)
    {
        if (!projectile.IsActive())
        {
            projectile.Spawn(position, direction, speed, damage);
            return true;  // Success!
        }
    }

    // Pool exhausted - all projectiles in use
    return false;
}

void ProjectilePool::Update(float deltaTime)
{
    for (auto& projectile : m_projectiles)
    {
        projectile.Update(deltaTime);
    }
}

void ProjectilePool::Render(const Matrix& view, const Matrix& projection)
{
    for (auto& projectile : m_projectiles)
    {
        projectile.Render(view, projection, m_sharedMesh.get());
    }
}

int ProjectilePool::GetActiveCount() const
{
    int count = 0;
    for (const auto& projectile : m_projectiles)
    {
        if (projectile.IsActive())
            count++;
    }
    return count;
}

void ProjectilePool::OnDeviceLost()
{
    m_sharedMesh.reset();
    m_projectiles.clear();
}