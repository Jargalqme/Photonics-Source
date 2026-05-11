#include "pch.h"
#include "Scenes/BossScene.h"
#include "Common/Camera.h"
#include "DeviceResources.h"
#include "Gameplay/Events/EventBus.h"
#include "Services/InputManager.h"
#include "Renderer.h"
#include "Gameplay/Combat/ShotIntent.h"
#include "Gameplay/Events/EventTypes.h"

namespace
{
    constexpr const char* RIFLE_FIRE_GROUP = "rifle_fire";
    constexpr float RIFLE_FIRE_VOLUME = 0.35f;
    constexpr float RIFLE_FIRE_PITCH_JITTER = 0.015f;

    void loadRifleFireAudio(AudioManager& audio)
    {
        if (audio.loadSoundGroupFromDirectory(
            RIFLE_FIRE_GROUP,
            GetAssetPath(L"Audio/Weapons"),
            L"rifle_fire_"))
        {
            return;
        }

        constexpr const char* fallbackName = "rifle_fire_fallback";
        if (audio.loadSound(fallbackName, GetAssetPath(L"Audio/shoot_sfx.wav")))
        {
            audio.addSoundToGroup(RIFLE_FIRE_GROUP, fallbackName);
        }
    }
}

BossScene::BossScene(SceneManager* sceneManager)
    : Scene("BossScene")
    , m_sceneManager(sceneManager)
{
}

BossScene::~BossScene()
{
}

// === 初期化 ===

void BossScene::initialize(SceneContext& context)
{
    Scene::initialize(context);

    // カメラ — アスペクト比はバックバッファではなくレンダー解像度から取る
    // （レンダーが 16:9、ウィンドウが 21:9 のとき投影は 16:9 のまま、合成側でレターボックス）
    m_camera = std::make_unique<Camera>();
    float aspectRatio = float(m_renderer->GetRenderWidth()) / float(m_renderer->GetRenderHeight());
    m_camera->setProjectionParameters(45.0f, aspectRatio, 0.1f, 1000.0f);

    // プレイヤー
    m_player = std::make_unique<Player>(*m_context);
    m_player->initialize();

    m_camera->setViewmodel(m_player->getViewmodel());

    // VFX
    m_particleSystem = std::make_unique<ParticleSystem>(m_deviceResources);
    m_particleSystem->initialize();

    m_bulletRenderer = std::make_unique<BulletRenderer>(m_deviceResources);
    m_bulletRenderer->initialize();

    m_tracers = std::make_unique<Tracers>(*m_context);
    m_tracers->initialize();

    // 敵
    m_boss = std::make_unique<Boss>(*m_context);
    m_boss->initialize();
    m_boss->setParticles(m_particleSystem.get());
    m_boss->setCamera(m_camera.get());

    m_player->getWeapon()->setDependencies(m_player->getViewmodel());

    // オーディオ
    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadSound("boost", GetAssetPath(L"Audio/boost_sfx.wav").c_str());
    loadRifleFireAudio(*m_audioManager);
    m_audioManager->loadSound("death", GetAssetPath(L"Audio/death_sfx.wav").c_str());
    m_audioManager->loadMusic("Gameplay", GetAssetPath(L"Audio/gameplay_music.wav").c_str());

    m_beatTracker = std::make_unique<BeatTracker>();
    m_beatTracker->setBPM(120.0f);
    m_beatTracker->setStartDelay(15.931f);
    m_beatTracker->setSongDuration(150.0f);
    m_beatTracker->setBeatCallback([this](int beat) {
        onBeat(beat);
    });

    // ワールド
    m_grid = std::make_unique<Grid>(m_deviceResources);
    m_grid->initialize();

    m_arenaFloor = std::make_unique<ArenaFloor>(m_deviceResources);
    m_arenaFloor->initialize();

    m_skybox = std::make_unique<Skybox>(m_deviceResources);
    m_skybox->initialize();

    // UI
    m_gameUI = std::make_unique<GameUI>();
    m_gameUI->initialize(m_deviceResources);  // m_deviceResources は Scene::initialize で設定済み
    m_gameUI->setBeatTracker(m_beatTracker.get());
    m_gameUI->setPlayer(m_player.get());
    m_gameUI->setBoss(m_boss.get());

#ifdef _DEBUG
    m_debugUI = std::make_unique<DebugUI>();
    m_debugUI->setCamera(m_camera.get());
    m_debugUI->setLightCycle(m_player.get());
    m_debugUI->setGrid(m_grid.get());
    m_debugUI->setSkybox(m_skybox.get());
    m_debugUI->setAudioManager(m_audioManager.get());
    m_debugUI->setBeatTracker(m_beatTracker.get());
    m_debugUI->setBulletPool(&m_bulletPool);
    m_debugUI->setBoss(m_boss.get());
    m_debugUI->setBloom(m_renderer->GetBloom());
#endif
}

// === シーン遷移 ===

void BossScene::enter()
{
    Scene::enter();
    EventBus::clear();
    m_debugMode = false;
    m_cursorVisibleBeforeDebug = false;
    if (m_context && m_context->input)
    {
        m_context->input->setCursorVisible(false);
    }

    // UI が自前で購読する（clear 後に再登録する必要があるためここで呼ぶ）
    m_gameUI->subscribeEvents();

    // 敵ヒット時 → 衝撃パーティクル
    EventBus::subscribe<DummyHitEvent>([this](const DummyHitEvent& event) {
        m_particleSystem->emit(event.position, Vector4(1.0f, 0.8f, 0.2f, 1.0f),
            IMPACT_PARTICLE_COUNT, IMPACT_PARTICLE_SPEED,
            IMPACT_PARTICLE_LIFE, IMPACT_PARTICLE_SPREAD);
    });

    EventBus::subscribe<WeaponShotEvent>([this](const WeaponShotEvent&) {
        m_camera->triggerShake(SHOOT_SHAKE_INTENSITY, SHOOT_SHAKE_DURATION);
        m_audioManager->playRandomSound(RIFLE_FIRE_GROUP, RIFLE_FIRE_VOLUME, RIFLE_FIRE_PITCH_JITTER);
        });

    // ヒットスキャン解決時 → トレーサー描画
    EventBus::subscribe<ShotResolvedEvent>([this](const ShotResolvedEvent& event) {
        m_tracers->spawn(event.tracerStart, event.hitPoint, event.color);
    });

    // 敵撃破時 → 爆発パーティクル + シェイク + 効果音
    EventBus::subscribe<DummyDiedEvent>([this](const DummyDiedEvent& event) {
        m_particleSystem->emit(event.position, Vector4(1.0f, 0.3f, 0.1f, 1.0f),
            DEATH_PARTICLE_COUNT, DEATH_PARTICLE_SPEED,
            DEATH_PARTICLE_LIFE, DEATH_PARTICLE_SPREAD);
        m_camera->triggerShake(KILL_SHAKE_INTENSITY, KILL_SHAKE_DURATION);
        m_audioManager->playSound("death", 0.2f);
    });

    EventBus::subscribe<PlayerDamagedEvent>([this](const PlayerDamagedEvent&) {
        m_camera->triggerShake(PLAYER_HIT_SHAKE_INTENSITY, PLAYER_HIT_SHAKE_DURATION);
    });

    EventBus::subscribe<BossDamagedEvent>([this](const BossDamagedEvent& event) {
        m_particleSystem->emit(event.position, Vector4(0.8f, 0.0f, 1.0f, 1.0f),
            BOSS_HIT_PARTICLE_COUNT, BOSS_HIT_PARTICLE_SPEED,
            BOSS_HIT_PARTICLE_LIFE, BOSS_HIT_PARTICLE_SPREAD);
    });

    EventBus::subscribe<BossDiedEvent>([this](const BossDiedEvent& event) {
        m_particleSystem->emit(event.position, Vector4(0.8f, 0.0f, 1.0f, 1.0f),
            BOSS_DEATH_PARTICLE_COUNT, BOSS_DEATH_PARTICLE_SPEED,
            BOSS_DEATH_PARTICLE_LIFE, BOSS_DEATH_PARTICLE_SPREAD);
        m_camera->triggerShake(BOSS_DEATH_SHAKE_INTENSITY, BOSS_DEATH_SHAKE_DURATION);
        m_audioManager->playSound("death", 0.3f);
    });

    // ゲーム状態リセット
    m_player->reset();

    m_boss->deactivate();

    for (int i = 0; i < m_bulletPool.getMaxBullets(); i++)
    {
        m_bulletPool.getBullets()[i].deactivate();
    }

    // Boss starts directly; Stage 4 replaces this with the intro FSM.
    m_boss->setPlayerTarget(m_player->getPositionPtr());
    m_boss->setBulletPool(&m_bulletPool);
    m_boss->activate();
    EventBus::publish(WaveChangedEvent{ 3 });

    // 戦闘ターゲットリスト構築 — Player + Boss
    m_combatTargets.clear();
    m_combatTargets.push_back(m_player.get());
    m_combatTargets.push_back(m_boss.get());

    // オーディオリセット
    m_beatTracker->reset();
    m_audioManager->playMusic("Gameplay");

    // カラーマスクリセット
    if (m_renderer)
    {
        ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
        if (colorMask)
        {
            colorMask->resetMask();
        }
    }

    // ブルーム有効化（exit() で disable されるため毎エントリで再有効化）
    m_renderer->GetBloom()->setEnabled(true);
}

void BossScene::exit()
{
    Scene::exit();
    EventBus::clear();
    m_audioManager->stopMusic();

    if (m_renderer)
    {
        ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
        if (colorMask)
        {
            colorMask->resetMask();
        }
    }

    if (m_renderer)
    {
        m_renderer->GetBloom()->setEnabled(false);
    }
}

void BossScene::finalize()
{
    // GPU リソース解放
    if (m_grid) { m_grid->finalize(); }
    if (m_arenaFloor) { m_arenaFloor->finalize(); }
    if (m_skybox) { m_skybox->finalize(); }
    if (m_player) { m_player->finalize(); }
    if (m_boss) { m_boss->finalize(); }
    if (m_particleSystem) { m_particleSystem->finalize(); }
    if (m_bulletRenderer) { m_bulletRenderer->finalize(); }
    if (m_tracers) { m_tracers->finalize(); }

    // オブジェクト破棄
    m_particleSystem.reset();
    m_bulletRenderer.reset();
    m_tracers.reset();
    m_camera.reset();
    m_player.reset();
    m_boss.reset();
    m_grid.reset();
    m_arenaFloor.reset();
    m_skybox.reset();
    m_beatTracker.reset();
    m_audioManager.reset();
    m_debugUI.reset();
}

// === 更新 ===

void BossScene::update(float deltaTime, InputManager* input)
{
    // === 入力 ===
    if (input->isKeyPressed(Keyboard::Keys::Escape))
    {
        PostQuitMessage(0);
    }

    if (!m_debugMode && input->isKeyPressed(Keyboard::Keys::Tab))
    {
        input->setCursorVisible(!input->isCursorVisible());
    }

#ifdef _DEBUG
    if (input->isKeyPressed(Keyboard::Keys::F3))
    {
        m_debugMode = !m_debugMode;
        if (m_debugMode)
        {
            m_cursorVisibleBeforeDebug = input->isCursorVisible();
            input->setCursorVisible(true);
        }
        else
        {
            input->setCursorVisible(m_cursorVisibleBeforeDebug);
        }
    }

    m_debugUI->setInputManager(input);
#endif

    // === ゲームプレイ系統（正準順: Player → Boss → Combat → Camera）===
    const bool victoryReached = m_boss->isActivated() && m_boss->isDead();

    // 1フレーム内の射撃インテントは Player 側で積み、Combat 側で解決して空にする
    std::vector<ShotIntent> shotIntents;
    if (!victoryReached)
    {
        PlayerInputState playerInput;
        playerInput.cursorHidden = !input->isCursorVisible();
        playerInput.lookDelta = input->getLookInput();
        playerInput.aimHeld = input->isRightMouseDown();
        playerInput.fireHeld = input->isLeftMouseDown();
        playerInput.reloadPressed = input->isKeyPressed(Keyboard::Keys::R);
        playerInput.jumpPressed = input->isKeyPressed(Keyboard::Keys::Space);
        if (input->isKeyDown(Keyboard::Keys::W))
        {
            playerInput.move.y += 1.0f;
        }
        if (input->isKeyDown(Keyboard::Keys::S))
        {
            playerInput.move.y -= 1.0f;
        }
        if (input->isKeyDown(Keyboard::Keys::A))
        {
            playerInput.move.x -= 1.0f;
        }
        if (input->isKeyDown(Keyboard::Keys::D))
        {
            playerInput.move.x += 1.0f;
        }

        m_playerSystem.update(*m_player, playerInput, shotIntents, deltaTime);
        if (m_boss->isActivated())
        {
            m_boss->update(deltaTime);
        }
        m_combatSystem.update(deltaTime, m_combatTargets, m_bulletPool, shotIntents);
    }

    EventBus::dispatchQueued();

    if (victoryReached)
    {
        m_sceneManager->transitionTo("Victory");
        return;
    }

    if (m_player->isDead())
    {
        m_sceneManager->transitionTo("GameOver");
        return;
    }

    m_camera->update(*m_player, deltaTime);

    // === サブシステム（UI / オーディオ / VFX / ワールド）===
    m_gameUI->update(deltaTime);
    m_audioManager->update();
    m_particleSystem->update(deltaTime);
    m_bulletRenderer->update(deltaTime);
    m_tracers->update(deltaTime);
    m_beatTracker->update(deltaTime);
    m_grid->setBeatPulse(m_beatTracker->getBeatProgress());
    m_grid->update();
    m_arenaFloor->update(deltaTime);
}

// === ヘルパー ===

void BossScene::onBeat(int beat)
{
    m_gameUI->triggerBeatFlash(beat);
}

// === 描画 ===

void BossScene::render()
{
    const auto view   = m_camera->getViewMatrix();
    const auto proj   = m_camera->getProjectionMatrix();
    const auto camPos = m_camera->getPosition();

    renderWorld(view, proj, camPos);      // グリッド + ボスメッシュ
    renderEffects(view, proj, camPos);    // 弾・パーティクル・トレーサー
    renderViewmodel(view, camPos);        // 深度クリア + 武器ビューモデル
    renderUI(view, proj);                 // GameUI + デバッグ
}

// === レンダーパス ===

void BossScene::renderWorld(const Matrix& view, const Matrix& proj, const Vector3& camPos)
{
    // ArenaFloor は将来用に保持（現状未使用）
    m_skybox->render(view, proj);
    //m_arenaFloor->render(view, proj);
    m_grid->render(view, proj);

    m_renderQueue.clear();
    m_boss->submitRender(m_renderQueue);
    m_renderer->ExecuteRenderCommands(m_renderQueue, view, proj, camPos);
}

void BossScene::renderEffects(const Matrix& view, const Matrix& proj, const Vector3& camPos)
{
    m_bulletRenderer->render(&m_bulletPool, view, proj, camPos);
    m_particleSystem->render(view, proj);
    m_tracers->render(view, proj, camPos);
}

void BossScene::renderViewmodel(const Matrix& view, const Vector3& camPos)
{
    m_renderer->BeginViewmodelPass();   // 深度クリアして常に最前面に描画

    m_renderQueue.clear();
    m_player->submitViewmodel(m_renderQueue, view);
    m_renderer->ExecuteRenderCommands(m_renderQueue, view, m_camera->getViewModelProjection(), camPos);
}

void BossScene::renderUI(const Matrix& view, const Matrix& proj)
{
    m_gameUI->render(view, proj);
#ifdef _DEBUG
    if (m_debugMode && m_debugUI)
    {
        m_debugUI->render();
    }
#endif
}
