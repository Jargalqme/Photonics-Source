#pragma once

#include "DeviceResources.h"
#include <SimpleMath.h>
#include <vector>
#include <memory>

class MusicManager;
class Player;
class Core;
class Enemy;
class BeamWeapon;

using namespace DirectX::SimpleMath;

class GameUI
{
public:
    GameUI();
    ~GameUI() = default;

    void initialize(DX::DeviceResources* deviceResources);
    void update(float deltaTime);
    void render();

    // Connect to game systems
    void setMusicManager(MusicManager* music) { m_musicManager = music; }
    void setPlayer(Player* player) { m_player = player; }
    void setCores(Core* r, Core* g, Core* b);
    void setEnemies(std::vector<std::unique_ptr<Enemy>>* enemies) { m_enemies = enemies; }
    void setBeamWeapon(BeamWeapon* beam) { m_beamWeapon = beam; }

    void triggerBeatFlash(int beat);

private:
    DX::DeviceResources* m_deviceResources = nullptr;

    // Game references (not owned, just watching)
    MusicManager* m_musicManager = nullptr;
    Player* m_player = nullptr;
    Core* m_coreRed = nullptr;
    Core* m_coreGreen = nullptr;
    Core* m_coreBlue = nullptr;
    BeamWeapon* m_beamWeapon = nullptr;
    std::vector<std::unique_ptr<Enemy>>* m_enemies = nullptr;

    // Edge glow effect
    float m_edgeGlowAlpha = 0.0f;
    float m_edgeGlowDecay = 5.0f;

    // Draw functions
    void drawBeatBar();
    void drawMinimap();
    void drawHealthBars();
    void drawBoostIndicator();
    void drawSongProgress();
    void drawCrosshair();

    //testing
    float m_beatPulseAlpha = 0.0f;

    ImU32 m_beatColor = IM_COL32(0, 255, 255, 255);
};