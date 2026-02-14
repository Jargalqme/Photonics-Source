#include "pch.h"
#include "CollisionManager.h"

void CollisionManager::Update(
	std::vector<std::unique_ptr<Enemy>>& enemies,
	Core* coreRed,
	Core* coreGreen,
	Core* coreBlue,
	DeathBeamPool* deathBeams,
	BeamWeapon* beamWeapon,
	bool isOnBeat)
{
	CheckEnemyVsCores(enemies, coreRed, coreGreen, coreBlue);
	CheckBeamVsEnemies(enemies, beamWeapon, deathBeams);
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
	DeathBeamPool* deathBeams)
{
	if (!beamWeapon || !beamWeapon->isActive()) return;

	Vector3 beamStart = beamWeapon->getStart();
	Vector3 beamDir = beamWeapon->getDirection();
	float maxRange = beamWeapon->getMaxRange();

	for (auto& enemy : enemies)
	{
		if (!enemy->IsActive()) continue;

		float hitDistance;
		if (CollisionSystem::CheckRaySphere(
			beamStart,
			beamDir,
			enemy->GetBoundingSphere(),
			hitDistance))
		{
			// Only count hit if within beam range
			if (hitDistance <= maxRange)
			{
				Vector3 deathPos = enemy->GetPosition();
				enemy->TakeDamage(1.0f);

				// Trigger death beam if enemy died
				if (!enemy->IsActive() && deathBeams)
				{
					deathBeams->Trigger(deathPos);
				}
			}
		}
	}
}
