#include "pch.h"
#include "CollisionManager.h"

void CollisionManager::Update(
	std::vector<std::unique_ptr<Enemy>>& enemies,
	Core* coreRed,
	Core* coreGreen,
	Core* coreBlue,
	DeathBeamPool* deathBeams,
	BeamWeapon* beamWeapon,
	bool isOnBeat,
	const Vector3& cameraPosition,
	const Vector3& cameraForward)
{
	CheckEnemyVsCores(enemies, coreRed, coreGreen, coreBlue);
	CheckBeamVsEnemies(enemies, beamWeapon, deathBeams, cameraPosition, cameraForward);
}

void CollisionManager::CheckEnemyVsCores(
	std::vector<std::unique_ptr<Enemy>>& enemies,
	Core* coreRed,
	Core* coreGreen,
	Core* coreBlue)
{
	Core* cores[] = { coreRed, coreGreen, coreBlue };

	for (auto& enemy : enemies)
	{
		if (!enemy->IsActive()) continue;

		for (Core* core : cores)
		{
			if (!core || !core->IsAlive()) continue;

			if (CollisionSystem::CheckSphereSphere(
				enemy->GetBoundingSphere(),
				core->GetBoundingSphere()))
			{
				core->TakeDamage(25.0f);
				enemy->Deactivate();
				break; // enemy gone, stop checking cores
			}
		}
	}
}

void CollisionManager::CheckBeamVsEnemies(
    std::vector<std::unique_ptr<Enemy>>& enemies,
    BeamWeapon* beamWeapon,
    DeathBeamPool* deathBeams,
    const Vector3& cameraPosition,
    const Vector3& cameraForward
)
{
    bool isActive = beamWeapon && beamWeapon->isCollisionActive();

    // Detect new shot (inactive -> active transition) and clear hit set
    if (isActive && !m_wasCollisionActive)
        m_hitEnemiesThisShot.clear();
    m_wasCollisionActive = isActive;

    if (!isActive) return;

    Vector3 rayStart = cameraPosition;
    Vector3 rayDir = cameraForward;
    rayDir.Normalize();

    float maxRange = beamWeapon->getMaxRange();

    for (auto& enemy : enemies)
    {
        if (!enemy->IsActive()) continue;
        if (m_hitEnemiesThisShot.count(enemy.get())) continue;

        float hitDistance;
        if (CollisionSystem::CheckRaySphere(
            rayStart, rayDir,
            enemy->GetBoundingSphere(),
            hitDistance))
        {
            if (hitDistance <= maxRange)
            {
                Vector3 deathPos = enemy->GetPosition();
                enemy->TakeDamage(10.0f);
                m_hitEnemiesThisShot.insert(enemy.get());

                if (!enemy->IsActive() && deathBeams)
                    deathBeams->Trigger(deathPos);
            }
        }
    }
}
