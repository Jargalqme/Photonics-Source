#pragma once
#include "EnemyTroops.h"
#include "Core.h"
#include "ProjectilePool.h"
#include "DeathBeamPool.h"
#include "BeamWeapon.h"
#include "CollisionSystem.h"
#include <vector>
#include <memory>
#include <unordered_set>

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
		bool isOnBeat,
		const Vector3& cameraPosition,
		const Vector3& cameraForward
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
		DeathBeamPool* deathBeams,
		const Vector3& cameraPosition,
		const Vector3& cameraForward
	);
	
	// Track enemies already hit during current shot
	std::unordered_set<Enemy*> m_hitEnemiesThisShot;
	bool m_wasCollisionActive = false;
};