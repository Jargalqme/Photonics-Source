#include "pch.h"
#include "GameScene.h"
#include "Camera.h"
#include "InputManager.h"
#include "Renderer.h"
#include "DeviceResources.h"

GameScene::GameScene(SceneManager* sceneManager)
	: Scene("GameScene")
	, m_sceneManager(sceneManager)
{
}

GameScene::~GameScene()
{
}

void GameScene::initialize(DX::DeviceResources* deviceResources)
{
	Scene::initialize(deviceResources);

	// Create camera
	m_camera = std::make_unique<Camera>();

	// Set up projection
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right) / float(size.bottom);
	m_camera->setProjectionParameters(45.0f, aspectRatio, 0.1f, 1000.0f);

	// Initialize game objects
	m_gridFloor = std::make_unique<GridFloor>(m_deviceResources);
	m_gridFloor->initialize();

	m_terrain = std::make_unique<Terrain>(m_deviceResources);
	m_terrain->initialize();

	m_player = std::make_unique<Player>(m_deviceResources);
	m_player->initialize();

	// Start with follow camera mode
	m_camera->setFollowTarget(m_player->getPositionPtr(), m_player->getRotationPtr());
	m_camera->setMode(CameraMode::Follow);

	m_playerController = std::make_unique<PlayerController>(m_player.get(), m_camera.get());

	m_enemyBoss = std::make_unique<EnemyBoss>(m_deviceResources);
	m_enemyBoss->initialize();

	m_enemies.reserve(MAX_ENEMIES);
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		auto enemy = std::make_unique<Enemy>(m_deviceResources);
		enemy->initialize();
		m_enemies.push_back(std::move(enemy));
	}

	m_coreRed = std::make_unique<Core>(m_deviceResources);
	m_coreRed->initialize();
	m_coreRed->setPosition(Vector3(0.0f, 1.0f, 85.0f));
	m_coreRed->setColor(Color(1.0f, 0.0f, 0.0f));
	m_coreRed->setHealth(150.0f);

	m_coreGreen = std::make_unique<Core>(m_deviceResources);
	m_coreGreen->initialize();
	m_coreGreen->setPosition(Vector3(-66.0f, 1.0f, -50.0f));
	m_coreGreen->setColor(Color(0.0f, 1.0f, 0.0f));
	m_coreGreen->setHealth(150.0f);

	m_coreBlue = std::make_unique<Core>(m_deviceResources);
	m_coreBlue->initialize();
	m_coreBlue->setPosition(Vector3(66.0f, 1.0f, -50.0f));
	m_coreBlue->setColor(Color(0.0f, 0.0f, 1.0f));
	m_coreBlue->setHealth(150.0f);

	m_enemySpawn = std::make_unique<EnemySpawn>();
	m_enemySpawn->initialize(&m_enemies, m_coreRed.get(), m_coreGreen.get(), m_coreBlue.get());

	m_beamWeapon = std::make_unique<BeamWeapon>(m_deviceResources);
	m_beamWeapon->initialize();

	m_animatedBillboard = std::make_unique<AnimatedBillboard>(m_deviceResources);
	m_animatedBillboard->setPosition(Vector3(0.0f, 90.0f, 700.0f));
	m_animatedBillboard->setSize(900.0f);
	m_animatedBillboard->setFrameRate(60.0f);
	m_animatedBillboard->initialize();

	m_deathBeams = std::make_unique<DeathBeamPool>(m_deviceResources);
	m_deathBeams->initialize(20);
	m_deathBeams->setHeight(30.0f);
	m_deathBeams->setColor(Color(0.0f, 0.0f, 0.0f));
	m_deathBeams->setDuration(0.5f);

	m_collisionManager = std::make_unique<CollisionManager>();

	m_audioManager = std::make_unique<AudioManager>();
	m_audioManager->initialize();
	m_audioManager->loadSound("boost", GetAssetPath(L"Audio/boost_sfx.wav").c_str());
	m_audioManager->loadSound("shoot", GetAssetPath(L"Audio/shoot_sfx.wav").c_str());
	m_audioManager->loadMusic("Gameplay", GetAssetPath(L"Audio/gameplay_music.wav").c_str());
	
	// === Music/Beat Setup ===
	m_musicManager = std::make_unique<MusicManager>();

	m_musicManager->setBPM(120.0f);
	m_musicManager->setStartDelay(15.931f);
	m_musicManager->setSongDuration(150.0f); // Original music duration: 305.0f
	

	// Set beat callback
	m_musicManager->setBeatCallback([this](int beat) {
		onBeat(beat);
	});

	m_gameUI = std::make_unique<GameUI>();
	m_gameUI->initialize(m_deviceResources);
	m_gameUI->setMusicManager(m_musicManager.get());
	m_gameUI->setPlayer(m_player.get());
	m_gameUI->setCores(m_coreRed.get(), m_coreGreen.get(), m_coreBlue.get());
	m_gameUI->setEnemies(&m_enemies);
	m_gameUI->setBeamWeapon(m_beamWeapon.get());

	m_debugUI = std::make_unique<DebugUI>();
	m_debugUI->setCamera(m_camera.get());
	m_debugUI->setLightCycle(m_player.get());
	m_debugUI->setGridFloor(m_gridFloor.get());
	m_debugUI->setTerrain(m_terrain.get());
	m_debugUI->setAudioManager(m_audioManager.get());
	m_debugUI->setMusicManager(m_musicManager.get());

	// === Floating Shapes ===
	auto ctx = m_deviceResources->GetD3DDeviceContext();

	// Create 15-20 shapes scattered in the sky
	for (int i = 0; i < 18; i++)
	{
		// Randomly choose shape type
		std::unique_ptr<GeometricPrimitive> shape;
		int type = rand() % 3;

		if (type == 0)
			shape = GeometricPrimitive::CreateIcosahedron(ctx, 3.0f + (rand() % 30) / 10.0f);
		else if (type == 1)
			shape = GeometricPrimitive::CreateOctahedron(ctx, 2.0f + (rand() % 20) / 10.0f);
		else
			shape = GeometricPrimitive::CreateDodecahedron(ctx, 2.5f + (rand() % 25) / 10.0f);

		m_floatingShapes.push_back(std::move(shape));

		// Random position in sky (spread around arena)
		float angle = (rand() % 360) * XM_PI / 180.0f;
		float distance = 60.0f + (rand() % 120);  // 60-180 units from center
		float height = 30.0f + (rand() % 80);     // 30-110 units high

		m_shapePositions.push_back(Vector3(
			cosf(angle) * distance,
			height,
			sinf(angle) * distance
		));

		// Random starting rotation
		m_shapeRotations.push_back(Vector3(
			(rand() % 628) / 100.0f,  // 0 to 2*PI
			(rand() % 628) / 100.0f,
			(rand() % 628) / 100.0f
		));

		// Random rotation speed
		m_shapeSpeeds.push_back(0.1f + (rand() % 30) / 100.0f);  // 0.1 to 0.4
	}
}

void GameScene::enter()
{
	Scene::enter();

	// reset spawn system
	if (m_enemySpawn) m_enemySpawn->reset();

	// reset player
	if (m_player)
		m_player->reset();

	// reset cores
	if (m_coreRed)   m_coreRed->reset(150.0f);
	if (m_coreGreen) m_coreGreen->reset(150.0f);
	if (m_coreBlue)  m_coreBlue->reset(150.0f);

	// deactivate all enemies
	for (auto& enemy : m_enemies)
		enemy->deactivate();

	// reset and start music
	if (m_musicManager) m_musicManager->reset();
	if (m_audioManager) m_audioManager->playMusic("Gameplay");

	// reset color mask
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
		if (colorMask)
			colorMask->resetMask();
	}
}

void GameScene::exit()
{
	Scene::exit();
	if (m_audioManager) m_audioManager->stopMusic();

	// Reset color mask
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
		if (colorMask)
			colorMask->resetMask();
	}
}

void GameScene::finalize()
{
	// Release GPU resources
	if (m_gridFloor) m_gridFloor->finalize();
	if (m_terrain) m_terrain->onDeviceLost();
	if (m_player) m_player->onDeviceLost();
	if (m_enemyBoss) m_enemyBoss->onDeviceLost();
	for (auto& enemy : m_enemies)
		enemy->onDeviceLost();
	if (m_coreRed) m_coreRed->onDeviceLost();
	if (m_coreGreen) m_coreGreen->onDeviceLost();
	if (m_coreBlue) m_coreBlue->onDeviceLost();
	if (m_deathBeams) m_deathBeams->onDeviceLost();
	if (m_beamWeapon) m_beamWeapon->finalize();

	// Destroy objects
	m_camera.reset();
	m_terrain.reset();
	m_player.reset();
	m_playerController.reset();
	m_enemyBoss.reset();
	m_enemies.clear();
	m_coreRed.reset();
	m_coreGreen.reset();
	m_coreBlue.reset();
	m_enemySpawn.reset();
	m_gridFloor.reset();
	m_musicManager.reset();
	m_audioManager.reset();
	m_collisionManager.reset();
	m_debugUI.reset();
	m_beamWeapon.reset();
	//m_animatedBillboard.reset();
}

void GameScene::update(float deltaTime, InputManager* input)
{
	// Handle scene-specific input
	if (input->isKeyPressed(Keyboard::Keys::Escape))
	{
		// Could push pause menu or exit
		PostQuitMessage(0);
	}

	// Toggle cursor for debug UI
	if (input->isKeyPressed(Keyboard::Keys::Tab))
	{
		input->setCursorVisible(!input->isCursorVisible());
	}

	// Toggle debug mode (F3)
	if (input->isKeyPressed(Keyboard::Keys::F3))
	{
		m_debugMode = !m_debugMode;
	}

	if (m_debugUI)
		m_debugUI->setInputManager(input);

	// Update subsystems
	updateGamePlay(deltaTime, input);
	m_playerController->update(deltaTime, input);
	m_camera->update(deltaTime, input);

	if (m_gameUI)
		m_gameUI->update(deltaTime);

	if (m_audioManager)
		m_audioManager->update();

	// Update beat tracking
	if (m_musicManager)
	{
		m_musicManager->update(deltaTime);
	}

	if (m_musicManager && m_gridFloor)
	{
		// always pulse - no phase restriction
		m_gridFloor->setBeatPulse(m_musicManager->getBeatProgress());
		m_gridFloor->update();
	}

	// Rumble decay
	if (m_rumbleTimer > 0.0f)
	{
		m_rumbleTimer -= deltaTime;
		if (m_rumbleTimer <= 0.0f)
		{
			input->stopVibration();
		}
	}

	if (m_deathBeams)
		m_deathBeams->update(deltaTime);

	if (m_beamWeapon)
	{
		m_beamWeapon->update(deltaTime);

		// zoom while holding right mouse (only when cursor hidden)
		m_camera->setAiming(!input->isCursorVisible() && input->isRightMouseDown());

		if (!input->isCursorVisible() && input->isLeftMousePressed() && m_beamWeapon->canFire())
		{
			Vector3 spawnPos = m_player->getPosition();
			spawnPos.y += 1.0f;

			Vector3 cameraPos = m_camera->getPosition();
			Vector3 cameraForward = m_camera->getForward();
			Vector3 aimPoint = cameraPos + cameraForward * 150.0f;
			Vector3 aimDirection = aimPoint - spawnPos;
			aimDirection.Normalize();

			if (m_audioManager) 
			m_audioManager->playSound("shoot");
			m_beamWeapon->fire(spawnPos, aimDirection);
			m_camera->triggerShake(2.0f, 0.1f);
		}
	}
	// TODO: Billboard based rendering model
	//if (m_animatedBillboard)
	//	m_animatedBillboard->update(deltaTime);

	// Update floating shapes rotation
	for (size_t i = 0; i < m_shapeRotations.size(); i++)
	{
		m_shapeRotations[i].x += m_shapeSpeeds[i] * deltaTime;
		m_shapeRotations[i].y += m_shapeSpeeds[i] * 0.7f * deltaTime;
	}
}

void GameScene::updateGamePlay(float deltaTime, InputManager* input)
{
	// Update enemy spawning
	if (m_enemySpawn)
		m_enemySpawn->update(deltaTime);

	// Update enemy retargeting
	updateEnemyRetargeting();

	for (auto& enemy : m_enemies)
	{
		if (enemy->isActive())
			enemy->update(deltaTime);
	}

	// Check all collisions
	bool isOnBeat = m_musicManager ? m_musicManager->isOnBeat() : false;
	m_collisionManager->update(
		m_enemies,
		m_coreRed.get(),
		m_coreGreen.get(),
		m_coreBlue.get(),
		m_deathBeams.get(),
		m_beamWeapon.get(),
		isOnBeat,
		m_camera->getPosition(),
		m_camera->getForward()
	);

	if (m_enemyBoss)
		m_enemyBoss->update(deltaTime);

	checkGameConditions();
}

void GameScene::updateEnemyRetargeting()
{
	// Retarget enemies if their target core is dead
	for (auto& enemy : m_enemies)
	{
		if (!enemy->isActive())
			continue;

		Vector3 targetPos = enemy->getTargetPosition();

		// Check if targeting dead core
		bool needsRetarget = false;

		if (!m_coreRed->isAlive() && targetPos == m_coreRed->getPosition())
			needsRetarget = true;
		else if (!m_coreGreen->isAlive() && targetPos == m_coreGreen->getPosition())
			needsRetarget = true;
		else if (!m_coreBlue->isAlive() && targetPos == m_coreBlue->getPosition())
			needsRetarget = true;

		// Retarget to random living core
		if (needsRetarget)
		{
			std::vector<Vector3> livingCores;
			if (m_coreRed->isAlive())   livingCores.push_back(m_coreRed->getPosition());
			if (m_coreGreen->isAlive()) livingCores.push_back(m_coreGreen->getPosition());
			if (m_coreBlue->isAlive())  livingCores.push_back(m_coreBlue->getPosition());

			if (!livingCores.empty())
			{
				int idx = rand() % livingCores.size();
				enemy->setTargetPosition(livingCores[idx]);
			}
		}
	}
}

void GameScene::checkGameConditions()
{
	// Check lose condition - player dead
	if (m_player && m_player->isDead())
	{
		m_sceneManager->transitionTo("GameOver");
		return;
	}

	// Check lose condition - all cores dead
	bool allCoresDead = !m_coreRed->isAlive() &&
		!m_coreGreen->isAlive() &&
		!m_coreBlue->isAlive();

	if (allCoresDead)
	{
		m_sceneManager->transitionTo("GameOver");
		return;
	}

	// Check win condition - song complete
	if (m_musicManager && m_musicManager->isSongComplete())
	{
		m_sceneManager->transitionTo("Victory");
		return;
	}

	// Update color mask based on dead cores
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();

		if (colorMask)
		{
			if (m_coreRed && !m_coreRed->isAlive())
				colorMask->disableRed();

			if (m_coreGreen && !m_coreGreen->isAlive())
				colorMask->disableGreen();

			if (m_coreBlue && !m_coreBlue->isAlive())
				colorMask->disableBlue();
		}
	}
}

void GameScene::onBeat(int beat)
{
	if (m_gameUI)
		m_gameUI->triggerBeatFlash(beat);
}

void GameScene::render(Renderer* renderer)
{
	m_renderer = renderer;

	// Render all game objects to the bloom render target
	if (m_gridFloor)
		m_gridFloor->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_terrain)
		m_terrain->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_player)
		m_player->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	// Render floating shapes
	for (size_t i = 0; i < m_floatingShapes.size(); i++)
	{
		Matrix world = Matrix::CreateRotationX(m_shapeRotations[i].x)
			* Matrix::CreateRotationY(m_shapeRotations[i].y)
			* Matrix::CreateRotationZ(m_shapeRotations[i].z)
			* Matrix::CreateTranslation(m_shapePositions[i]);

		// Glowing cyan/purple color
		Color shapeColor = (i % 2 == 0)
			? Color(0.0f, 0.8f, 1.0f, 0.7f)   // Cyan
			: Color(0.6f, 0.0f, 1.0f, 0.7f);  // Purple

		m_floatingShapes[i]->Draw(world,
			m_camera->getViewMatrix(),
			m_camera->getProjectionMatrix(),
			shapeColor);
	}

	if (m_enemyBoss)
		m_enemyBoss->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	for (auto& enemy : m_enemies)
	{
		if (enemy->isActive())
			enemy->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}

	if (m_coreRed)
		m_coreRed->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_coreGreen)
		m_coreGreen->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_coreBlue)
		m_coreBlue->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_deathBeams)
		m_deathBeams->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());

	if (m_beamWeapon)
		m_beamWeapon->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());
	
	//if (m_animatedBillboard)
		//m_animatedBillboard->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());

	if (m_gameUI)
		m_gameUI->render();

	if (m_debugMode && m_debugUI)
		m_debugUI->render();
}


