#pragma once
#include "Common/Transform.h"

// 前方宣言
class Player;
class Grid;
class Skybox;
class AudioManager;
class BeatTracker;
class Camera;
class InputManager;
class BulletPool;
class Boss;
class Bloom;

class DebugUI
{
public:
    DebugUI() = default;

    // ゲームオブジェクトのポインタ設定（オブジェクト生成後に1回呼ぶ）
    void setCamera(Camera* camera) { m_camera = camera; }
    void setLightCycle(Player* player) { m_player = player; }
    void setGrid(Grid* grid) { m_grid = grid; }
    void setSkybox(Skybox* skybox) { m_skybox = skybox; }
    void setAudioManager(AudioManager* audio) { m_audioManager = audio; }
    void setBeatTracker(BeatTracker* music) { m_beatTracker = music; }
    void setFireRatePtr(float* fireRate) { m_fireRate = fireRate; }
    void setInputManager(InputManager* input) { m_input = input; }
    void setBulletPool(BulletPool* pool) { m_bulletPool = pool; }
    void setBoss(Boss* boss) { m_boss = boss; }
    void setBloom(Bloom* bloom) { m_bloom = bloom; }

    /// @brief デバッグパネルを描画する
    void render();

private:
    // ゲームオブジェクトへのポインタ（所有しない）
    Camera* m_camera = nullptr;
    Player* m_player = nullptr;
    Grid* m_grid = nullptr;
    Skybox* m_skybox = nullptr;
    AudioManager* m_audioManager = nullptr;
    BeatTracker* m_beatTracker = nullptr;
    float* m_fireRate = nullptr;
    InputManager* m_input = nullptr;
    BulletPool* m_bulletPool = nullptr;
    Boss* m_boss = nullptr;
    Bloom* m_bloom = nullptr;

    bool m_showThirds = false;
};
