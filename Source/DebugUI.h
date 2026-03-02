#pragma once
#include "Transform.h"

// Forward declarations (avoid includes in header)
class Player;
class GridFloor;
class Terrain;
class ProjectilePool;
class AudioManager;
class MusicManager;
class Camera;
class InputManager;

class DebugUI
{
public:
    DebugUI() = default;

    // Set pointers to game objects (call once after objects exist)
    void setCamera(Camera* camera) { m_camera = camera; }
    void setLightCycle(Player* player) { m_player = player; }
    void setGridFloor(GridFloor* grid) { m_gridFloor = grid; }
    void setTerrain(Terrain* terrain) { m_terrain = terrain; }
    void setProjectilePool(ProjectilePool* pool) { m_projectilePool = pool; }
    void setAudioManager(AudioManager* audio) { m_audioManager = audio; }
    void setMusicManager(MusicManager* music) { m_musicManager = music; }
    void setFireRatePtr(float* fireRate) { m_fireRate = fireRate; }
    void setInputManager(InputManager* input) { m_input = input; }

    void render();

private:
    // Pointers to game objects (not owned)
    Camera* m_camera = nullptr;
    Player* m_player = nullptr;
    GridFloor* m_gridFloor = nullptr;
    Terrain* m_terrain = nullptr;
    ProjectilePool* m_projectilePool = nullptr;
    AudioManager* m_audioManager = nullptr;
    MusicManager* m_musicManager = nullptr;
    float* m_fireRate = nullptr;
    InputManager* m_input = nullptr;
};
