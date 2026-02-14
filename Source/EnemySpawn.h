#pragma once
#include "EnemyTroops.h"
#include "Core.h"
#include <vector>
#include <memory>

class EnemySpawn
{
public:
	EnemySpawn();
	~EnemySpawn() = default;

	void initialize(std::vector<std::unique_ptr<Enemy>>* enemyPool,
		Core* coreRed, Core* coreGreen, Core* coreBlue);
	void update(float deltaTime);
	void reset();

private:

	// Spawn logic
	void attemptSpawn();
	Vector3 selectSpawnTarget();
	Vector3 getSpawnPosition();
	void spawnEnemy(EnemyType type, const Vector3& spawnPos, const Vector3& target);

	// Patterns
	void applySpawnPattern(Enemy* enemy);

	// References
	std::vector<std::unique_ptr<Enemy>>* m_enemyPool;
	Core* m_coreRed;
	Core* m_coreGreen;
	Core* m_coreBlue;

	// Spawn state
	float m_spawnTimer;
	float m_spawnInterval;
	float m_gameTime;

	// Spawn configuration
	static constexpr float INITIAL_SPAWN_INTERVAL = 2.0f;
	static constexpr float MIN_SPAWN_INTERVAL = 0.5f;
	static constexpr float DIFFICULTY_RAMP_TIME = 60.0f;
	static constexpr float SPAWN_RADIUS = 30.0f;
	static constexpr float WAVE_MOVEMENT_CHANCE = 0.4f;
};