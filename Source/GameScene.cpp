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

void GameScene::Initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;

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
	m_terrain->Initialize();

	m_lightCycle = std::make_unique<LightCycle>(m_deviceResources);
	m_lightCycle->Initialize();

	// Start with follow camera mode
	m_camera->setFollowTarget(m_lightCycle->GetPositionPtr(), m_lightCycle->GetRotationPtr());
	m_camera->setMode(CameraMode::Follow);

	m_tower = std::make_unique<Tower>(m_deviceResources);
	m_tower->Initialize();

	m_enemies.reserve(MAX_ENEMIES);
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		auto enemy = std::make_unique<Enemy>(m_deviceResources);
		enemy->Initialize();
		m_enemies.push_back(std::move(enemy));
	}



	m_coreRed = std::make_unique<Core>(m_deviceResources);
	m_coreRed->Initialize();
	m_coreRed->SetPosition(Vector3(0.0f, 1.0f, 85.0f));
	m_coreRed->SetColor(Color(1.0f, 0.0f, 0.0f));
	m_coreRed->SetHealth(150.0f);

	m_coreGreen = std::make_unique<Core>(m_deviceResources);
	m_coreGreen->Initialize();
	m_coreGreen->SetPosition(Vector3(-66.0f, 1.0f, -50.0f));
	m_coreGreen->SetColor(Color(0.0f, 1.0f, 0.0f));
	m_coreGreen->SetHealth(150.0f);

	m_coreBlue = std::make_unique<Core>(m_deviceResources);
	m_coreBlue->Initialize();
	m_coreBlue->SetPosition(Vector3(66.0f, 1.0f, -50.0f));
	m_coreBlue->SetColor(Color(0.0f, 0.0f, 1.0f));
	m_coreBlue->SetHealth(150.0f);

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
	m_deathBeams->Initialize(20);
	m_deathBeams->SetHeight(30.0f);
	m_deathBeams->SetColor(Color(0.0f, 0.0f, 0.0f));
	m_deathBeams->SetDuration(0.5f);

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
		OnBeat(beat);
	});

	m_gameUI = std::make_unique<GameUI>();
	m_gameUI->initialize(m_deviceResources);
	m_gameUI->setMusicManager(m_musicManager.get());
	m_gameUI->setPlayer(m_lightCycle.get());
	m_gameUI->setCores(m_coreRed.get(), m_coreGreen.get(), m_coreBlue.get());
	m_gameUI->setEnemies(&m_enemies);
	m_gameUI->setBeamWeapon(m_beamWeapon.get());

	m_debugUI = std::make_unique<DebugUI>();
	m_debugUI->setCamera(m_camera.get());
	m_debugUI->setLightCycle(m_lightCycle.get());
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

void GameScene::Enter()
{
	Scene::Enter();

	// reset spawn system
	if (m_enemySpawn) m_enemySpawn->reset();

	// reset player
	if (m_lightCycle)
		m_lightCycle->Reset();

	// reset cores
	if (m_coreRed)   m_coreRed->Reset(150.0f);
	if (m_coreGreen) m_coreGreen->Reset(150.0f);
	if (m_coreBlue)  m_coreBlue->Reset(150.0f);

	// deactivate all enemies
	for (auto& enemy : m_enemies)
		enemy->Deactivate();

	// reset and start music
	if (m_musicManager) m_musicManager->reset();
	if (m_audioManager) m_audioManager->playMusic("Gameplay");

	// reset color mask
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
		if (colorMask)
			colorMask->ResetMask();
	}
}

void GameScene::Exit()
{
	Scene::Exit();
	if (m_audioManager) m_audioManager->stopMusic();

	// Reset color mask
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
		if (colorMask)
			colorMask->ResetMask();
	}
}

void GameScene::Cleanup()
{
	// Clean up resources (explicitly)
	m_camera.reset();
	m_terrain.reset();
	m_lightCycle.reset();
	m_tower.reset();
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

void GameScene::Update(float deltaTime, InputManager* input)
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
		m_showCursor = !m_showCursor;
		input->setMouseMode(m_showCursor ? Mouse::MODE_ABSOLUTE : Mouse::MODE_RELATIVE);
	}

	// Toggle debug mode (F3)
	if (input->isKeyPressed(Keyboard::Keys::F3))
	{
		m_debugMode = !m_debugMode;
	}

	if (m_debugUI)
		m_debugUI->setShowCursor(m_showCursor);

	// Update subsystems
	UpdateGamePlay(deltaTime, input);
	UpdateCamera(deltaTime, input);

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
		m_deathBeams->Update(deltaTime);

	if (m_beamWeapon)
	{
		m_beamWeapon->update(deltaTime);

		// zoom while holding right mouse
		m_camera->setZoom(input->isRightMouseDown());

		if (input->isLeftMousePressed() && m_beamWeapon->canFire())
		{
			Vector3 spawnPos = m_lightCycle->GetPosition();
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

void GameScene::UpdateCamera(float deltaTime, InputManager* input)
{
	// Toggle camera mode
	if (input->isKeyPressed(Keyboard::Keys::F1))
	{
		m_camera->setMode(CameraMode::Free);
	}
	if (input->isKeyPressed(Keyboard::Keys::F2))
	{
		m_camera->setFollowTarget(m_lightCycle->GetPositionPtr(), m_lightCycle->GetRotationPtr());
		m_camera->setMode(CameraMode::Follow);
	}

	// Update camera (skip when cursor visible)
	if (!m_showCursor)
	{
		m_camera->update(deltaTime, input);
	}
}

void GameScene::UpdateGamePlay(float deltaTime, InputManager* input)
{
	if (m_lightCycle)
	{
		if (m_camera->getMode() == CameraMode::Follow)
		{
			// Get camera directions
			Vector3 cameraForward = m_camera->getForward();
			cameraForward.y = 0.0f;
			cameraForward.Normalize();

			Vector3 cameraRight = m_camera->getRight();
			cameraRight.y = 0.0f;
			cameraRight.Normalize();

			// Build movement direction from WASD
			Vector3 moveDirection = Vector3::Zero;

			if (input->isKeyDown(Keyboard::Keys::W))
				moveDirection += cameraForward;
			if (input->isKeyDown(Keyboard::Keys::S))
				moveDirection -= cameraForward;
			if (input->isKeyDown(Keyboard::Keys::A))
				moveDirection -= cameraRight;
			if (input->isKeyDown(Keyboard::Keys::D))
				moveDirection += cameraRight;

			// Gamepad support
			Vector2 stick = input->getGamePadLeftStick();
			if (stick.LengthSquared() > 0.01f)
			{
				moveDirection += cameraForward * stick.y;
				moveDirection += cameraRight * stick.x;
			}

			// Movement & Facing
			bool isMoving = moveDirection.LengthSquared() > 0.001f;
			bool isShooting = input->isLeftMousePressed() || input->isRightMousePressed();

			if (isMoving || isShooting)
			{
				m_lightCycle->MoveInDirection(moveDirection, cameraForward, deltaTime);
			}
			else
			{
				m_lightCycle->UpdateIdle(deltaTime);
			}

			// Jump
			if (input->isKeyPressed(Keyboard::Keys::Space))
			{
				m_lightCycle->Jump();
			}
		}
		else
		{
			// Free camera mode
			if (input->isKeyDown(Keyboard::Keys::Up))
				m_lightCycle->Accelerate(deltaTime);
			if (input->isKeyDown(Keyboard::Keys::Down))
				m_lightCycle->Brake(deltaTime);
			if (input->isKeyDown(Keyboard::Keys::Left))
				m_lightCycle->Turn(1.0f, deltaTime);
			if (input->isKeyDown(Keyboard::Keys::Right))
				m_lightCycle->Turn(-1.0f, deltaTime);

			m_lightCycle->Update(deltaTime);
		}
	}

	// Boost (separate from jump if using beat system)
	if (input->isKeyPressed(Keyboard::Keys::LeftShift))  // Changed to Shift since Space is jump
	{
		if (m_musicManager && m_musicManager->isOnBeat())
		{
			m_lightCycle->ActivateBoost();

			if (m_audioManager) m_audioManager->playSound("boost");

			input->setVibration(0.7f, 0.4f);
			m_rumbleTimer = 0.2f;
		}
	}

	// Update enemy spawning
	if (m_enemySpawn)
		m_enemySpawn->update(deltaTime);

	// Update enemy retargeting
	UpdateEnemyRetargeting();

	for (auto& enemy : m_enemies)
	{
		if (enemy->IsActive())
			enemy->Update(deltaTime);
	}

	// Check all collisions
	bool isOnBeat = m_musicManager ? m_musicManager->isOnBeat() : false;
	m_collisionManager->Update(
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

	if (m_tower)
		m_tower->Update(deltaTime);

	CheckGameConditions();
}

void GameScene::UpdateEnemyRetargeting()
{
	// Retarget enemies if their target core is dead
	for (auto& enemy : m_enemies)
	{
		if (!enemy->IsActive())
			continue;

		Vector3 targetPos = enemy->GetTargetPosition();

		// Check if targeting dead core
		bool needsRetarget = false;

		if (!m_coreRed->IsAlive() && targetPos == m_coreRed->GetPosition())
			needsRetarget = true;
		else if (!m_coreGreen->IsAlive() && targetPos == m_coreGreen->GetPosition())
			needsRetarget = true;
		else if (!m_coreBlue->IsAlive() && targetPos == m_coreBlue->GetPosition())
			needsRetarget = true;

		// Retarget to random living core
		if (needsRetarget)
		{
			std::vector<Vector3> livingCores;
			if (m_coreRed->IsAlive())   livingCores.push_back(m_coreRed->GetPosition());
			if (m_coreGreen->IsAlive()) livingCores.push_back(m_coreGreen->GetPosition());
			if (m_coreBlue->IsAlive())  livingCores.push_back(m_coreBlue->GetPosition());

			if (!livingCores.empty())
			{
				int idx = rand() % livingCores.size();
				enemy->SetTargetPosition(livingCores[idx]);
			}
		}
	}
}

void GameScene::CheckGameConditions()
{
	// Check lose condition - player dead
	if (m_lightCycle && m_lightCycle->IsDead())
	{
		m_sceneManager->TransitionTo("GameOver");
		return;
	}

	// Check lose condition - all cores dead
	bool allCoresDead = !m_coreRed->IsAlive() &&
		!m_coreGreen->IsAlive() &&
		!m_coreBlue->IsAlive();

	if (allCoresDead)
	{
		m_sceneManager->TransitionTo("GameOver");
		return;
	}

	// Check win condition - song complete
	if (m_musicManager && m_musicManager->isSongComplete())
	{
		m_sceneManager->TransitionTo("Victory");
		return;
	}

	// Update color mask based on dead cores
	if (m_renderer)
	{
		ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();

		if (colorMask)
		{
			if (m_coreRed && !m_coreRed->IsAlive())
				colorMask->DisableRed();

			if (m_coreGreen && !m_coreGreen->IsAlive())
				colorMask->DisableGreen();

			if (m_coreBlue && !m_coreBlue->IsAlive())
				colorMask->DisableBlue();
		}
	}
}

void GameScene::OnBeat(int beat)
{
	if (m_gameUI)
		m_gameUI->triggerBeatFlash(beat);
}

void GameScene::Render(Renderer* renderer)
{
	m_renderer = renderer;

	// Render all game objects to the bloom render target
	if (m_gridFloor)
		m_gridFloor->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_terrain)
		m_terrain->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_lightCycle)
		m_lightCycle->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

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

	if (m_tower)
		m_tower->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	for (auto& enemy : m_enemies)
	{
		if (enemy->IsActive())
			enemy->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
	}

	if (m_coreRed)
		m_coreRed->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_coreGreen)
		m_coreGreen->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_coreBlue)
		m_coreBlue->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());

	if (m_deathBeams)
		m_deathBeams->Render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());

	if (m_beamWeapon)
		m_beamWeapon->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());
	
	//if (m_animatedBillboard)
		//m_animatedBillboard->render(m_camera->getViewMatrix(), m_camera->getProjectionMatrix(), m_camera->getPosition());

	if (m_gameUI)
		m_gameUI->render();

	if (m_debugMode && m_debugUI)
		m_debugUI->render();
}

void GameScene::OnWindowSizeChanged(int width, int height)
{
	// Update camera aspect ratio
	if (m_camera)
	{
		m_camera->setProjectionParameters(45.0f, float(width) / float(height), 0.1f, 1000.0f);
	}
}

void GameScene::OnDeviceLost()
{
	// Handle device lost
	if (m_gridFloor) m_gridFloor->finalize();

	if (m_terrain) m_terrain->OnDeviceLost();

	if (m_lightCycle) m_lightCycle->OnDeviceLost();

	if (m_tower) m_tower->OnDeviceLost();

	for (auto& enemy : m_enemies)
	{
		enemy->OnDeviceLost();
	}

	if (m_coreRed) m_coreRed->OnDeviceLost();

	if (m_coreGreen) m_coreGreen->OnDeviceLost();

	if (m_coreBlue) m_coreBlue->OnDeviceLost();

	if (m_deathBeams) m_deathBeams->OnDeviceLost();

	//if (m_animatedBillboard) m_animatedBillboard->finalize();

	if (m_beamWeapon) m_beamWeapon->finalize();
}

