#include "pch.h"
#include "CollisionManager.h"

void CollisionManager::update(
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
	checkEnemyVsCores(enemies, coreRed, coreGreen, coreBlue);
	checkBeamVsEnemies(enemies, beamWeapon, deathBeams, cameraPosition, cameraForward);
}

void CollisionManager::checkEnemyVsCores(
	std::vector<std::unique_ptr<Enemy>>& enemies,
	Core* coreRed,
	Core* coreGreen,
	Core* coreBlue)
{
	Core* cores[] = { coreRed, coreGreen, coreBlue };

	for (auto& enemy : enemies)
	{
		if (!enemy->isActive()) continue;

		for (Core* core : cores)
		{
			if (!core || !core->isAlive()) continue;

			if (CollisionSystem::checkSphereSphere(
				enemy->getBoundingSphere(),
				core->getBoundingSphere()))
			{
				core->takeDamage(25.0f);
				enemy->deactivate();
				break; // enemy gone, stop checking cores
			}
		}
	}
}

void CollisionManager::checkBeamVsEnemies(
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
        if (!enemy->isActive()) continue;
        if (m_hitEnemiesThisShot.count(enemy.get())) continue;

        float hitDistance;
        if (CollisionSystem::checkRaySphere(
            rayStart, rayDir,
            enemy->getBoundingSphere(),
            hitDistance))
        {
            if (hitDistance <= maxRange)
            {
                Vector3 deathPos = enemy->getPosition();
                enemy->takeDamage(10.0f);
                m_hitEnemiesThisShot.insert(enemy.get());

                if (!enemy->isActive() && deathBeams)
                    deathBeams->trigger(deathPos);
            }
        }
    }
}
