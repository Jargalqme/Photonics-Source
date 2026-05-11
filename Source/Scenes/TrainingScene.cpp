#include "pch.h"
#include "Scenes/TrainingScene.h"
#include "Common/Camera.h"
#include "DeviceResources.h"
#include "Gameplay/Combat/ShotIntent.h"
#include "Gameplay/Events/EventBus.h"
#include "Gameplay/Events/EventTypes.h"
#include "Renderer.h"
#include "Services/InputManager.h"
#include "Render/ImportedModelCache.h"
#include <imgui.h>
#include <limits>

namespace
{
    constexpr const char* RIFLE_FIRE_GROUP = "rifle_fire";
    constexpr float RIFLE_FIRE_VOLUME = 0.35f;
    constexpr float RIFLE_FIRE_PITCH_JITTER = 0.015f;

#ifdef _DEBUG
    constexpr bool SHOW_IMPORTED_RIFLE_WORLD_PREVIEW = false;
    constexpr const char* IMPORTED_RIFLE_PREVIEW_PATH = "Assets/Weapons/Rifle/rifle.glb";
    constexpr float IMPORTED_RIFLE_PREVIEW_TARGET_LENGTH = 2.5f;
    const Vector3 IMPORTED_RIFLE_PREVIEW_POSITION(0.0f, 1.2f, 6.0f);

    Matrix makeImportedRiflePreviewWorld(const ImportedModel& model)
    {
        const auto& vertices = model.data().vertices;
        if (vertices.empty())
        {
            return Matrix::CreateTranslation(IMPORTED_RIFLE_PREVIEW_POSITION);
        }

        const float maxFloat = std::numeric_limits<float>::max();
        Vector3 minBounds(maxFloat, maxFloat, maxFloat);
        Vector3 maxBounds(-maxFloat, -maxFloat, -maxFloat);

        for (const auto& vertex : vertices)
        {
            minBounds.x = std::min(minBounds.x, vertex.position.x);
            minBounds.y = std::min(minBounds.y, vertex.position.y);
            minBounds.z = std::min(minBounds.z, vertex.position.z);
            maxBounds.x = std::max(maxBounds.x, vertex.position.x);
            maxBounds.y = std::max(maxBounds.y, vertex.position.y);
            maxBounds.z = std::max(maxBounds.z, vertex.position.z);
        }

        const Vector3 size = maxBounds - minBounds;
        const float longestSide = std::max({ size.x, size.y, size.z });
        const float scale = longestSide > 0.001f
            ? IMPORTED_RIFLE_PREVIEW_TARGET_LENGTH / longestSide
            : 1.0f;
        const Vector3 center = (minBounds + maxBounds) * 0.5f;

        return Matrix::CreateTranslation(-center)
            * Matrix::CreateScale(scale)
            * Matrix::CreateRotationY(XMConvertToRadians(180.0f))
            * Matrix::CreateTranslation(IMPORTED_RIFLE_PREVIEW_POSITION);
    }
#endif

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

TrainingScene::TrainingScene(SceneManager* sceneManager)
    : Scene("Training")
    , m_sceneManager(sceneManager)
{
}

TrainingScene::~TrainingScene() = default;

// === 初期化 ===

void TrainingScene::initialize(SceneContext& context)
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
    m_player->getWeapon()->setDependencies(m_player->getViewmodel());

    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    loadRifleFireAudio(*m_audioManager);

    // VFX
    m_particleSystem = std::make_unique<ParticleSystem>(m_deviceResources);
    m_particleSystem->initialize();

    m_bulletRenderer = std::make_unique<BulletRenderer>(m_deviceResources);
    m_bulletRenderer->initialize();

    m_tracers = std::make_unique<Tracers>(*m_context);
    m_tracers->initialize();

    // ワールド
    m_grid = std::make_unique<Grid>(m_deviceResources);
    m_grid->initialize();

    // ダミー（最大数を確保し、enter() で必要数だけアクティブ化）
    allocateDummies();

    // UI
    m_gameUI = std::make_unique<GameUI>();
    m_gameUI->initialize(m_deviceResources);
    m_gameUI->setPlayer(m_player.get());
    m_gameUI->setDummies(&m_dummies);
    m_gameUI->setShowWaveIndicator(false);

#ifdef _DEBUG
    m_debugUI = std::make_unique<DebugUI>();
    m_debugUI->setCamera(m_camera.get());
    m_debugUI->setLightCycle(m_player.get());
    m_debugUI->setGrid(m_grid.get());
    m_debugUI->setBulletPool(&m_bulletPool);
    m_debugUI->setBloom(m_renderer->GetBloom());
    m_debugUI->setAudioManager(m_audioManager.get());
#endif
}

void TrainingScene::allocateDummies()
{
    m_dummies.clear();
    m_dummies.reserve(MAX_DUMMIES);
    for (int i = 0; i < MAX_DUMMIES; i++)
    {
        m_dummies.push_back(std::make_unique<Dummy>(*m_context));
        m_dummies.back()->initialize();
    }
}

// === シーン遷移 ===

void TrainingScene::enter()
{
    Scene::enter();
    EventBus::clear();
    m_debugMode = false;
    m_cursorVisibleBeforeDebug = false;
    if (m_context && m_context->input)
    {
        m_context->input->setCursorVisible(false);
    }

    m_gameUI->subscribeEvents();

    // ダミー命中 → 衝撃パーティクル
    EventBus::subscribe<DummyHitEvent>([this](const DummyHitEvent& event) {
        m_particleSystem->emit(event.position, Vector4(1.0f, 0.8f, 0.2f, 1.0f),
            IMPACT_PARTICLE_COUNT, IMPACT_PARTICLE_SPEED,
            IMPACT_PARTICLE_LIFE, IMPACT_PARTICLE_SPREAD);
    });

    // 射撃 → カメラシェイク
    EventBus::subscribe<WeaponShotEvent>([this](const WeaponShotEvent&) {
        m_camera->triggerShake(SHOOT_SHAKE_INTENSITY, SHOOT_SHAKE_DURATION);
        m_audioManager->playRandomSound(RIFLE_FIRE_GROUP, RIFLE_FIRE_VOLUME, RIFLE_FIRE_PITCH_JITTER);
    });

    // ヒットスキャン解決 → トレーサー
    EventBus::subscribe<ShotResolvedEvent>([this](const ShotResolvedEvent& event) {
        m_tracers->spawn(event.tracerStart, event.hitPoint, event.color);
    });

    // ダミー撃破 → 爆発パーティクル + シェイク
    EventBus::subscribe<DummyDiedEvent>([this](const DummyDiedEvent& event) {
        m_particleSystem->emit(event.position, Vector4(1.0f, 0.3f, 0.1f, 1.0f),
            DEATH_PARTICLE_COUNT, DEATH_PARTICLE_SPEED,
            DEATH_PARTICLE_LIFE, DEATH_PARTICLE_SPREAD);
        m_camera->triggerShake(KILL_SHAKE_INTENSITY, KILL_SHAKE_DURATION);
    });

    // プレイヤーリセット
    m_player->reset();

    // 弾プールを念のためクリア（前シーン残留分対策）
    for (int i = 0; i < m_bulletPool.getMaxBullets(); i++)
    {
        m_bulletPool.getBullets()[i].deactivate();
    }

    // ダミー初期化 — m_dummyCount 体だけ初期位置にスポーン
    resetAllDummies();
    applyDummyParameters();

    // 戦闘ターゲット = ダミーのみ。Player を含めない = 永久無敵。
    m_combatTargets.clear();
    for (auto& dummy : m_dummies)
    {
        m_combatTargets.push_back(dummy.get());
    }

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

void TrainingScene::exit()
{
    Scene::exit();
    EventBus::clear();

    if (m_renderer)
    {
        ColorMaskEffect* colorMask = m_renderer->GetColorMaskEffect();
        if (colorMask)
        {
            colorMask->resetMask();
        }
        m_renderer->GetBloom()->setEnabled(false);
    }
}

void TrainingScene::finalize()
{
    if (m_grid) { m_grid->finalize(); }
    if (m_player) { m_player->finalize(); }
    if (m_particleSystem) { m_particleSystem->finalize(); }
    if (m_bulletRenderer) { m_bulletRenderer->finalize(); }
    if (m_tracers) { m_tracers->finalize(); }
    for (auto& dummy : m_dummies)
    {
        dummy->finalize();
    }

    m_dummies.clear();
    m_particleSystem.reset();
    m_bulletRenderer.reset();
    m_tracers.reset();
    m_camera.reset();
    m_player.reset();
    m_grid.reset();
    m_audioManager.reset();
    m_debugUI.reset();
}

// === ダミーヘルパー ===

Vector3 TrainingScene::dummyGridPosition(int index) const
{
    const int col = index % DUMMY_COLUMNS;
    const int row = index / DUMMY_COLUMNS;
    const float x = (float(col) - (DUMMY_COLUMNS - 1) * 0.5f) * DUMMY_SPACING_X;
    const float z = DUMMY_BASE_Z + float(row) * DUMMY_SPACING_Z;
    return Vector3(x, 0.0f, z);
}

void TrainingScene::resetAllDummies()
{
    for (int i = 0; i < MAX_DUMMIES; i++)
    {
        if (i < m_dummyCount)
        {
            m_dummies[i]->spawn(dummyGridPosition(i));
        }
        else
        {
            m_dummies[i]->deactivate();
        }
    }
}

void TrainingScene::applyDummyParameters()
{
    for (auto& dummy : m_dummies)
    {
        dummy->setMoving(m_dummyMoving, m_dummyMoveAmplitude, m_dummyMoveFrequency);
        dummy->setInvulnerable(m_dummyInvulnerable);
        dummy->setRespawnDelay(m_dummyRespawnDelay);
    }
}

// === 更新 ===

void TrainingScene::update(float deltaTime, InputManager* input)
{
    // === 入力 ===
    if (input->isKeyPressed(Keyboard::Keys::Escape))
    {
        m_sceneManager->transitionTo("MainMenu");
        return;
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

    // === ゲームプレイ系統（正準順: Player → Dummy → Combat → Camera）===
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

    std::vector<ShotIntent> shotIntents;
    m_playerSystem.update(*m_player, playerInput, shotIntents, deltaTime);

    for (auto& dummy : m_dummies)
    {
        dummy->update(deltaTime);
    }

    m_combatSystem.update(deltaTime, m_combatTargets, m_bulletPool, shotIntents);

    EventBus::dispatchQueued();

    m_camera->update(*m_player, deltaTime);

    // === サブシステム ===
    m_gameUI->update(deltaTime);
    m_audioManager->update();
    m_particleSystem->update(deltaTime);
    m_bulletRenderer->update(deltaTime);
    m_tracers->update(deltaTime);
    m_grid->update();
}

// === 描画 ===

void TrainingScene::render()
{
    const auto view   = m_camera->getViewMatrix();
    const auto proj   = m_camera->getProjectionMatrix();
    const auto camPos = m_camera->getPosition();

    renderWorld(view, proj, camPos);      // グリッド + ダミーメッシュ
    renderEffects(view, proj, camPos);    // 弾・パーティクル・トレーサー
    renderViewmodel(view, camPos);        // 深度クリア + 武器ビューモデル
    renderUI(view, proj);                 // GameUI + 訓練パネル + デバッグ
}

// === レンダーパス ===

void TrainingScene::renderWorld(const Matrix& view, const Matrix& proj, const Vector3& camPos)
{
    m_grid->render(view, proj);

    m_renderQueue.clear();
    for (const auto& dummy : m_dummies)
    {
        dummy->submitRender(m_renderQueue);
    }

#ifdef _DEBUG
    if constexpr (SHOW_IMPORTED_RIFLE_WORLD_PREVIEW)
    {
        if (m_context && m_context->importedModels)
        {
            if (const ImportedModel* rifle = m_context->importedModels->get(IMPORTED_RIFLE_PREVIEW_PATH))
            {
                ImportedModelCommand command;
                command.model = rifle;
                command.world = makeImportedRiflePreviewWorld(*rifle);
                command.color = Color(1.0f, 1.0f, 1.0f, 1.0f);
                m_renderQueue.submit(command);
            }
        }
    }
#endif

    m_renderer->ExecuteRenderCommands(m_renderQueue, view, proj, camPos);
}

void TrainingScene::renderEffects(const Matrix& view, const Matrix& proj, const Vector3& camPos)
{
    m_bulletRenderer->render(&m_bulletPool, view, proj, camPos);
    m_particleSystem->render(view, proj);
    m_tracers->render(view, proj, camPos);
}

void TrainingScene::renderViewmodel(const Matrix& view, const Vector3& camPos)
{
    m_renderer->BeginViewmodelPass();   // 深度クリアして常に最前面に描画

    m_renderQueue.clear();
    m_player->submitViewmodel(m_renderQueue, view);
    m_renderer->ExecuteRenderCommands(m_renderQueue, view, m_camera->getViewModelProjection(), camPos);
}

void TrainingScene::renderUI(const Matrix& view, const Matrix& proj)
{
    m_gameUI->render(view, proj);
    renderTrainingPanel();
#ifdef _DEBUG
    if (m_debugMode && m_debugUI)
    {
        m_debugUI->render();
    }
#endif
}

// === トレーニングパネル（常時表示） ===

void TrainingScene::renderTrainingPanel()
{
    ImGui::SetNextWindowPos(ImVec2(20.0f, 220.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 430.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Training");

#ifdef _DEBUG
    ImGui::Text("Esc: menu | Tab: cursor | F3: debug");
#else
    ImGui::Text("Esc: menu | Tab: cursor");
#endif
    ImGui::Separator();

    if (ImGui::SliderInt("Dummy Count", &m_dummyCount, 1, MAX_DUMMIES))
    {
        resetAllDummies();
        applyDummyParameters();
    }

    if (ImGui::Button("Reset All"))
    {
        resetAllDummies();
        applyDummyParameters();
    }

    ImGui::Separator();
    ImGui::Text("Movement");

    if (ImGui::Checkbox("Moving", &m_dummyMoving))
    {
        applyDummyParameters();
    }
    if (ImGui::SliderFloat("Amplitude", &m_dummyMoveAmplitude, 0.0f, 8.0f))
    {
        applyDummyParameters();
    }
    if (ImGui::SliderFloat("Frequency", &m_dummyMoveFrequency, 0.0f, 5.0f))
    {
        applyDummyParameters();
    }

    ImGui::Separator();
    ImGui::Text("Behavior");

    if (ImGui::Checkbox("Invulnerable", &m_dummyInvulnerable))
    {
        applyDummyParameters();
    }
    if (ImGui::SliderFloat("Respawn Delay", &m_dummyRespawnDelay, 0.0f, 5.0f))
    {
        applyDummyParameters();
    }

    if (m_player && m_player->hasImportedRifleViewmodel())
    {
        ImportedRifleViewmodelSettings& rifleSettings =
            m_player->getImportedRifleViewmodelSettings();

        ImGui::Separator();
        ImGui::Text("Imported Rifle");
        ImGui::DragFloat("Length", &rifleSettings.targetLength, 0.005f, 0.01f, 5.0f, "%.3f");
        ImGui::DragFloat3("Position", &rifleSettings.position.x, 0.005f, -5.0f, 5.0f, "%.3f");
        ImGui::DragFloat3("Rotation", &rifleSettings.rotationDegrees.x, 0.25f, -360.0f, 360.0f, "%.1f");

        if (ImGui::Button("Reset Rifle Viewmodel"))
        {
            m_player->resetImportedRifleViewmodelSettings();
        }
    }

    ImGui::End();
}
