#pragma once

// 基底
#include "Scenes/Scene.h"

// ゲームオブジェクト
#include "Gameplay/Player.h"
#include "Gameplay/Dummy.h"
#include "Render/Grid.h"

// 戦闘
#include "Gameplay/BulletPool.h"
#include "Gameplay/Combat/ICombatTarget.h"

// ゲームプレイ系統
#include "Gameplay/CombatSystem.h"
#include "Gameplay/PlayerSystem.h"

// VFX
#include "Render/BulletRenderer.h"
#include "Render/ParticleSystem.h"
#include "Render/RenderCommandQueue.h"
#include "Render/Tracers.h"

// UI
#include "UI/GameUI.h"
#include "UI/DebugUI.h"
#include "Services/AudioManager.h"

// シーン管理
#include "Scenes/SceneManager.h"

#include <memory>
#include <vector>

class Camera;

class TrainingScene : public Scene
{
public:
    TrainingScene(SceneManager* sceneManager);
    ~TrainingScene() override;

    // --- シーンインターフェース ---
    void initialize(SceneContext& context) override;
    void enter() override;
    void exit() override;
    void finalize() override;

    void update(float deltaTime, InputManager* input) override;
    void render() override;

private:
    // --- 定数 ---
    // ダミー配置
    static constexpr int   MAX_DUMMIES         = 30;
    static constexpr int   DEFAULT_DUMMY_COUNT = 12;
    static constexpr int   DUMMY_COLUMNS       = 5;
    static constexpr float DUMMY_SPACING_X     = 4.0f;
    static constexpr float DUMMY_SPACING_Z     = 3.5f;
    static constexpr float DUMMY_BASE_Z        = 12.0f;

    // パーティクル演出パラメータ
    static constexpr int   IMPACT_PARTICLE_COUNT  = 30;
    static constexpr float IMPACT_PARTICLE_SPEED  = 6.0f;
    static constexpr float IMPACT_PARTICLE_LIFE   = 0.4f;
    static constexpr float IMPACT_PARTICLE_SPREAD = 0.8f;

    static constexpr int   DEATH_PARTICLE_COUNT  = 200;
    static constexpr float DEATH_PARTICLE_SPEED  = 8.0f;
    static constexpr float DEATH_PARTICLE_LIFE   = 1.5f;
    static constexpr float DEATH_PARTICLE_SPREAD = 1.0f;

    // カメラシェイク
    static constexpr float KILL_SHAKE_INTENSITY  = 0.15f;
    static constexpr float KILL_SHAKE_DURATION   = 0.2f;

    // --- シーン管理 ---
    SceneManager* m_sceneManager;

    // カメラ
    std::unique_ptr<Camera> m_camera;

    // プレイヤー（CombatTarget には登録しない = 永久無敵）
    std::unique_ptr<Player> m_player;

    // ダミー集合
    std::vector<std::unique_ptr<Dummy>> m_dummies;
    std::vector<ICombatTarget*> m_combatTargets;

    // ゲームプレイ系統
    PlayerSystem m_playerSystem;
    BulletPool m_bulletPool;          // 訓練ではほぼ未使用（ダミーは射撃しない）
    CombatSystem m_combatSystem;

    // VFX
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<BulletRenderer> m_bulletRenderer;
    std::unique_ptr<Tracers> m_tracers;
    RenderCommandQueue m_renderQueue;
    std::unique_ptr<AudioManager> m_audioManager;

    // ワールド
    std::unique_ptr<Grid> m_grid;

    // UI
    std::unique_ptr<GameUI> m_gameUI;
    std::unique_ptr<DebugUI> m_debugUI;
    bool m_debugMode = false;
    bool m_cursorVisibleBeforeDebug = false;

    // --- ダミー調整パラメータ（ImGui で操作） ---
    int   m_dummyCount         = DEFAULT_DUMMY_COUNT;
    bool  m_dummyMoving        = false;
    float m_dummyMoveAmplitude = 3.0f;
    float m_dummyMoveFrequency = 1.0f;
    bool  m_dummyInvulnerable  = false;
    float m_dummyRespawnDelay  = 1.0f;

    // --- 内部メソッド ---
    void allocateDummies();
    void resetAllDummies();
    void applyDummyParameters();
    Vector3 dummyGridPosition(int index) const;

    // --- レンダーパス（render() がこの順で呼ぶ）---
    void renderWorld(const Matrix& view, const Matrix& proj, const Vector3& camPos);
    void renderEffects(const Matrix& view, const Matrix& proj, const Vector3& camPos);
    void renderViewmodel(const Matrix& view, const Vector3& camPos);
    void renderUI(const Matrix& view, const Matrix& proj);
    void renderTrainingPanel();
};
