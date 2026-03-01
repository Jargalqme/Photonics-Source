#pragma once
#include "Scene.h"
#include "GridFloor.h"
#include "Terrain.h"
#include "EnemyTower.h"
#include "EnemyTroops.h"
#include "EnemySpawn.h"
#include "Core.h"
#include "Player.h"
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
	void Initialize(DX::DeviceResources* deviceResources) override;
	void Enter() override;
	void Exit() override;
	void Cleanup() override;

	void Update(float deltaTime, InputManager* input) override;
	void Render(Renderer* renderer) override;

	void OnWindowSizeChanged(int width, int height) override;
	void OnDeviceLost() override;

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
	std::unique_ptr<Tower> m_tower;

	// Player
	std::unique_ptr<LightCycle> m_lightCycle;
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
	bool m_showCursor = false;
	bool m_debugMode = false;

	// Private helper methods
	void UpdateCamera(float deltaTime, InputManager* input);
	void UpdateGamePlay(float deltaTime, InputManager* input);
	void UpdateEnemyRetargeting();
	void CheckGameConditions();
	void OnBeat(int beat);
};