#pragma once
#include "EnemyTroops.h"
#include "Core.h"
#include "ProjectilePool.h"
#include "DeathBeamPool.h"
#include "BeamWeapon.h"
#include "CollisionSystem.h"
#include <vector>
#include <memory>

class CollisionManager
{
public:
	void Update(
		std::vector<std::unique_ptr<Enemy>>& enemies,
		Core* coreRed,
		Core* coreGreen,
		Core* coreBlue,
		DeathBeamPool* deathBeams,
		BeamWeapon* beamWeapon,
		bool isOnBeat
	);
private:
	void CheckEnemyVsCores(
		std::vector<std::unique_ptr<Enemy>>& enemies,
		Core* coreRed,
		Core* coreGreen,
		Core* coreBlue
	);

	void CheckBeamVsEnemies(
		std::vector<std::unique_ptr<Enemy>>& enemies,
		BeamWeapon* beamWeapon,
		DeathBeamPool* deathBeams
	);
};