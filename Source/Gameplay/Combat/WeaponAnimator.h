#pragma once
#include "Common/Spring.h"
#include <SimpleMath.h>

struct WeaponOffset
{
    DirectX::SimpleMath::Vector3 pos;      // right, down, forward
    DirectX::SimpleMath::Vector3 rotDeg;   // pitch, yaw, roll（度）
};

struct WeaponGameParams
{
    float moveSpeed = 0.0f;   // 水平移動速度 m/s
    bool  grounded  = true;
};

// DebugUI でライブ調整するフィール値（ImGui スライダー用）
struct WeaponTuning
{
    // Recoil
    float recoilImpulseZ     = -8.0f;

    // Aim punch
    float aimPunchImpulseDeg = -6.0f;

    // Bob
    float bobFrequency       = 8.0f;
    float bobAmplitudeX      = 0.02f;
    float bobAmplitudeY      = 0.015f;

    // Sway
    float swayGain           = 0.003f;

    // Landing
    float landGainY          = 0.012f;
    float landGainPitch      = 1.0f;
};

class WeaponAnimator
{
public:
    void update(float dt);
    void onFire();
    void onLand(float fallSpeed);
    void reset();

    void setGameParams(const WeaponGameParams& p) { m_game = p; }
    void addLookDelta(float yawDeg, float pitchDeg);
    WeaponOffset  getOffset()    const { return m_composed; }
    WeaponTuning* getTuningPtr()       { return &m_tuning; }

private:
    WeaponOffset computeRecoil(float dt);
    WeaponOffset computeAimPunch(float dt);
    WeaponOffset computeBob(float dt);
    WeaponOffset computeSway(float dt);
    WeaponOffset computeLanding(float dt);

    // Recoil
    Spring1D m_recoilZ;

    // Aim punch
    Spring1D m_aimPitch { .zeta = 0.6f, .omega = 30.0f };

    // Bob
    float m_bobPhase   = 0.0f;
    float m_bobAmount  = 0.0f;
    float m_bobFalloff = 5.0f;   // 停止時のフェード速度

    // Sway — 視点回転に対する慣性（カメラから deg 差分を受け取る）
    Spring1D m_swayX { .zeta = 0.7f, .omega = 12.0f };
    Spring1D m_swayY { .zeta = 0.7f, .omega = 12.0f };
    float m_swayMaxKick = 20.0f;     // 瞬時スナップ上限（度/frame）

    // Landing kick — 着地衝撃
    Spring1D m_landY     { .zeta = 0.5f, .omega = 12.0f };   // 位置Y（下へ沈む）
    Spring1D m_landPitch { .zeta = 0.6f, .omega = 14.0f };   // 視点ピッチ（下へ）
    float m_landMaxSpeed = 30.0f;    // 衝撃上限
    float m_landMinSpeed = 1.0f;     // 小落下は無視

    WeaponTuning     m_tuning;
    WeaponGameParams m_game;
    WeaponOffset     m_composed;
};
