#pragma once
#include "Scene.h"
#include "GridFloor.h"
#include "Terrain.h"
#include "EnemyBoss.h"
#include "EnemyTroops.h"
#include "EnemySpawn.h"
#include "Core.h"
#include "Player.h"
#include "PlayerController.h"
#include "DeathBeamPool.h"
#include "ProjectilePool.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "MusicManager.h"
#include "CollisionSystem.h"
#include "CollisionManager.h"
#include "GameUI.h"
#include "DebugUI.h"
#include "BeamWeapon.h"
#include "AnimatedBillboard.h"
#include <memory>
#include <cstdint>

class Camera;

class GameScene : public Scene {
public:
	GameScene(SceneManager* sceneManager);
	~GameScene() override;

	// Scene interface
	void initialize(DX::DeviceResources* deviceResources) override;
	void enter() override;
	void exit() override;
	void finalize() override;

	void update(float deltaTime, InputManager* input) override;
	void render(Renderer* renderer) override;

private:
	// Core systems
	std::unique_ptr<Camera> m_camera;
	Renderer* m_renderer = nullptr;
	SceneManager* m_sceneManager;

	// Managers
	std::unique_ptr<AudioManager> m_audioManager;
	std::unique_ptr<MusicManager> m_musicManager;
	std::unique_ptr<CollisionManager> m_collisionManager;

	// World objects
	std::unique_ptr<GridFloor> m_gridFloor;
	std::unique_ptr<Terrain> m_terrain;
	std::unique_ptr<EnemyBoss> m_enemyBoss;

	// Player
	std::unique_ptr<Player> m_player;
	std::unique_ptr<PlayerController> m_playerController;
	std::unique_ptr<BeamWeapon> m_beamWeapon;

	// Objectives (Cores)
	std::unique_ptr<Core> m_coreRed;
	std::unique_ptr<Core> m_coreGreen;
	std::unique_ptr<Core> m_coreBlue;

	// Enemies
	std::vector<std::unique_ptr<Enemy>> m_enemies;
	static constexpr uint32_t MAX_ENEMIES = 50;
	std::unique_ptr<EnemySpawn> m_enemySpawn;

	// Projectiles & Effects
	std::unique_ptr<DeathBeamPool> m_deathBeams;
	std::unique_ptr<AnimatedBillboard> m_animatedBillboard;

	// UI
	std::unique_ptr<GameUI> m_gameUI;
	std::unique_ptr<DebugUI> m_debugUI;

	// Visual effects (floating shapes)
	std::vector<std::unique_ptr<DirectX::GeometricPrimitive>> m_floatingShapes;
	std::vector<DirectX::SimpleMath::Vector3> m_shapePositions;
	std::vector<DirectX::SimpleMath::Vector3> m_shapeRotations;
	std::vector<float> m_shapeSpeeds;

	// Debug
	float m_rumbleTimer = 0.0f;
	bool m_debugMode = false;

	// Private helper methods
	void updateGamePlay(float deltaTime, InputManager* input);
	void updateEnemyRetargeting();
	void checkGameConditions();
	void onBeat(int beat);
};