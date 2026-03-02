#include "pch.h"
#include "EnemySpawn.h"
#include <random>

EnemySpawn::EnemySpawn()
	: m_enemyPool(nullptr)
	, m_coreRed(nullptr)
	, m_coreGreen(nullptr)
	, m_coreBlue(nullptr)
	, m_spawnTimer(0.0f)
	, m_spawnInterval(INITIAL_SPAWN_INTERVAL)
	, m_gameTime(0.0f)
{
}

void EnemySpawn::initialize(std::vector<std::unique_ptr<Enemy>>* enemyPool,
	Core* coreRed, Core* coreGreen, Core* coreBlue)
{
	m_enemyPool = enemyPool;
	m_coreRed = coreRed;
	m_coreGreen = coreGreen;
	m_coreBlue = coreBlue;
}

void EnemySpawn::update(float deltaTime)
{
	m_gameTime += deltaTime;
	m_spawnTimer += deltaTime;

	// Gradually decrease spawn interval over time (increasing difficulty)
	float difficultyProgress = std::min(m_gameTime / DIFFICULTY_RAMP_TIME, 1.0f);
	m_spawnInterval = INITIAL_SPAWN_INTERVAL - (INITIAL_SPAWN_INTERVAL - MIN_SPAWN_INTERVAL) * difficultyProgress;

	if (m_spawnTimer >= m_spawnInterval)
	{
		m_spawnTimer = 0.0f;
		attemptSpawn();
	}
}

void EnemySpawn::reset()
{
	m_spawnTimer = 0.0f;
	m_spawnInterval = INITIAL_SPAWN_INTERVAL;
	m_gameTime = 0.0f;
}

void EnemySpawn::attemptSpawn()
{
	if (!m_enemyPool)
		return;

	// Select a target core (only alive ones)
	Vector3 targetPos = selectSpawnTarget();

	// Check if we found a valid target
	if (targetPos == Vector3::Zero)
		return;  // No alive cores

	// Get spawn position
	Vector3 spawnPos = getSpawnPosition();

	// Spawn the enemy
	spawnEnemy(EnemyType::Troop_Type_1, spawnPos, targetPos);
}

Vector3 EnemySpawn::selectSpawnTarget()
{
	// Build list of alive cores
	std::vector<Vector3> aliveCores;

	if (m_coreRed && m_coreRed->isAlive())
		aliveCores.push_back(m_coreRed->getPosition());

	if (m_coreGreen && m_coreGreen->isAlive())
		aliveCores.push_back(m_coreGreen->getPosition());

	if (m_coreBlue && m_coreBlue->isAlive())
		aliveCores.push_back(m_coreBlue->getPosition());

	// If no alive cores, return zero vector
	if (aliveCores.empty())
		return Vector3::Zero;

	// Pick a random alive core
	int index = rand() % aliveCores.size();
	return aliveCores[index];
}

Vector3 EnemySpawn::getSpawnPosition()
{
	// Random angle around the arena
	float angle = (rand() / (float)RAND_MAX) * XM_2PI;

	return Vector3(
		cosf(angle) * SPAWN_RADIUS,
		1.0f,
		sinf(angle) * SPAWN_RADIUS
	);
}

void EnemySpawn::spawnEnemy(EnemyType type, const Vector3& spawnPos, const Vector3& target)
{
	// Find an inactive enemy in the pool
	for (auto& enemy : *m_enemyPool)
	{
		if (!enemy->isActive())
		{
			// Spawn the enemy
			enemy->spawn(type, spawnPos, target);

			// Apply spawn patterns (wave movement)
			applySpawnPattern(enemy.get());
			return;
		}
	}
}

void EnemySpawn::applySpawnPattern(Enemy* enemy)
{
	if (!enemy)
		return;

	// 40% chance to apply wave movement
	float random = (rand() / (float)RAND_MAX);

	if (random < WAVE_MOVEMENT_CHANCE)
	{
		// Vary wave parameters for visual variety
		float amplitude = 2.0f + (rand() % 30) / 10.0f;  // 2.0 to 5.0
		float frequency = 1.5f + (rand() % 20) / 10.0f;  // 1.5 to 3.5

		enemy->setWaveMovement(true, amplitude, frequency);
	}
	else
	{
		enemy->setWaveMovement(false);
	}
}