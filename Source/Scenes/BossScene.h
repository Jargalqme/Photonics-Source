#pragma once

// 基底
#include "Scenes/Scene.h"

// ゲームオブジェクト
#include "Gameplay/Player.h"
#include "Gameplay/Boss.h"
#include "Render/Grid.h"
#include "Render/Skybox.h"
#include "Render/ArenaFloor.h"

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

// オーディオ
#include "Services/AudioManager.h"
#include "Services/BeatTracker.h"

// UI
#include "UI/GameUI.h"
#include "UI/DebugUI.h"

// シーン管理
#include "Scenes/SceneManager.h"

#include <memory>
#include <cstdint>

class Camera;

class BossScene : public Scene
{
public:
    BossScene(SceneManager* sceneManager);
    ~BossScene() override;

    // --- シーンインターフェース ---
    void initialize(SceneContext& context) override;
    void enter() override;
    void exit() override;
    void finalize() override;

    void update(float deltaTime, InputManager* input) override;
    void render() override;

private:
    // --- 定数 ---
    // パーティクル演出パラメータ
    static constexpr int   IMPACT_PARTICLE_COUNT   = 30;
    static constexpr float IMPACT_PARTICLE_SPEED   = 6.0f;
    static constexpr float IMPACT_PARTICLE_LIFE    = 0.4f;
    static constexpr float IMPACT_PARTICLE_SPREAD  = 0.8f;

    static constexpr int   DEATH_PARTICLE_COUNT    = 200;
    static constexpr float DEATH_PARTICLE_SPEED    = 8.0f;
    static constexpr float DEATH_PARTICLE_LIFE     = 1.5f;
    static constexpr float DEATH_PARTICLE_SPREAD   = 1.0f;

    // カメラシェイク
    static constexpr float KILL_SHAKE_INTENSITY    = 0.15f;
    static constexpr float KILL_SHAKE_DURATION     = 0.2f;
    static constexpr float PLAYER_HIT_SHAKE_INTENSITY = 0.35f;
    static constexpr float PLAYER_HIT_SHAKE_DURATION  = 0.15f;

    static constexpr int   BOSS_HIT_PARTICLE_COUNT = 24;
    static constexpr float BOSS_HIT_PARTICLE_SPEED = 6.0f;
    static constexpr float BOSS_HIT_PARTICLE_LIFE  = 0.4f;
    static constexpr float BOSS_HIT_PARTICLE_SPREAD = 0.8f;

    static constexpr int   BOSS_DEATH_PARTICLE_COUNT = 150;
    static constexpr float BOSS_DEATH_PARTICLE_SPEED = 12.0f;
    static constexpr float BOSS_DEATH_PARTICLE_LIFE  = 2.0f;
    static constexpr float BOSS_DEATH_PARTICLE_SPREAD = 1.0f;
    static constexpr float BOSS_DEATH_SHAKE_INTENSITY = 1.0f;
    static constexpr float BOSS_DEATH_SHAKE_DURATION  = 1.0f;

    // --- シーン管理 ---
    SceneManager* m_sceneManager;

    // カメラ
    std::unique_ptr<Camera> m_camera;

    // プレイヤー
    std::unique_ptr<Player> m_player;

    // 敵
    std::unique_ptr<Boss> m_boss;

    // 戦闘ターゲット — Player + Boss を ICombatTarget として CombatSystem に渡す
    std::vector<ICombatTarget*> m_combatTargets;

    // ゲームプレイ系統
    PlayerSystem m_playerSystem;

    // 戦闘
    BulletPool m_bulletPool;
    CombatSystem m_combatSystem;

    // VFX
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<BulletRenderer> m_bulletRenderer;
    std::unique_ptr<Tracers> m_tracers;
    RenderCommandQueue m_renderQueue;

    // オーディオ
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<BeatTracker> m_beatTracker;

    // ワールド
    std::unique_ptr<Grid> m_grid;
    std::unique_ptr<ArenaFloor> m_arenaFloor;
    std::unique_ptr<Skybox> m_skybox;

    // UI
    std::unique_ptr<GameUI> m_gameUI;
    std::unique_ptr<DebugUI> m_debugUI;
    bool m_debugMode = false;
    bool m_cursorVisibleBeforeDebug = false;

    // 入力フィードバック
    float m_hitstopTimer = 0.0f;

    // --- 内部メソッド ---
    void onBeat(int beat);

    // --- レンダーパス（render() がこの順で呼ぶ）---
    void renderWorld(const Matrix& view, const Matrix& proj, const Vector3& camPos);
    void renderEffects(const Matrix& view, const Matrix& proj, const Vector3& camPos);
    void renderViewmodel(const Matrix& view, const Vector3& camPos);
    void renderUI(const Matrix& view, const Matrix& proj);
};
