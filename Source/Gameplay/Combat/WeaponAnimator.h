#pragma once

#include <SimpleMath.h>

struct WeaponAnimationInput
{
    float deltaTime = 0.0f;

    // Camera/player look change for this frame, in degrees.
    // This is visual lag only; gameplay aim still comes from the camera.
    DirectX::SimpleMath::Vector2 lookDeltaDegrees = {};

    // Player horizontal movement speed normalized to 0..1.
    float moveSpeed01 = 0.0f;

    bool isGrounded = true;
    bool isAiming = false;
};

struct WeaponAnimationOutput
{
    // Camera-local pose: +X right, +Y up, +Z forward.
    DirectX::SimpleMath::Vector3 position = {};

    // Degrees keep tuning readable in DebugUI: pitch, yaw, roll.
    DirectX::SimpleMath::Vector3 rotationDegrees = {};
};

struct WeaponAnimationTuning
{
    bool enableAds = true;
    bool enableSway = true;
    bool enableBob = true;
    bool enableRecoil = true;

    DirectX::SimpleMath::Vector3 hipPosition = { 0.30f, -0.25f, 0.70f };
    DirectX::SimpleMath::Vector3 hipRotationDegrees = { 0.0f, 0.0f, 0.0f };
    DirectX::SimpleMath::Vector3 adsPosition = { 0.0f, -0.085f, 0.35f };
    DirectX::SimpleMath::Vector3 adsRotationDegrees = { 0.0f, 0.0f, 0.0f };
    float adsBlendSpeed = 10.0f;

    float swayMaxLookDeltaDegrees = 20.0f;
    float swayPositionAmount = 0.003f;
    float swayRotationAmountDegrees = 0.18f;
    float swayReturnSpeed = 12.0f;

    float bobSpeed = 8.0f;
    float bobHorizontalAmount = 0.020f;
    float bobVerticalAmount = 0.015f;
    float bobRollDegrees = 0.60f;
    float bobBlendSpeed = 8.0f;

    float recoilKickback = 0.080f;
    float recoilPitchDegrees = 4.0f;
    float recoilReturnSpeed = 14.0f;
    float recoilMaxKickback = 0.160f;
    float recoilMaxPitchDegrees = 10.0f;
};

class WeaponAnimator
{
public:
    void update(const WeaponAnimationInput& input);
    void onWeaponFired();
    void reset();

    WeaponAnimationOutput getOutput() const { return m_composed; }
    WeaponAnimationTuning* getTuningPtr() { return &m_tuning; }

private:
    void updateAds(float deltaTime, bool isAiming);

    WeaponAnimationOutput computeBasePose() const;
    WeaponAnimationOutput computeSway(const DirectX::SimpleMath::Vector2& lookDeltaDegrees, float deltaTime);
    WeaponAnimationOutput computeBob(float moveSpeed01, bool isGrounded, float deltaTime);
    WeaponAnimationOutput computeRecoil(float deltaTime);

    float m_adsAlpha = 0.0f;

    DirectX::SimpleMath::Vector3 m_swayPosition = {};
    DirectX::SimpleMath::Vector3 m_swayRotationDegrees = {};

    float m_bobPhase = 0.0f;
    float m_bobAmount = 0.0f;

    DirectX::SimpleMath::Vector3 m_recoilPosition = {};
    DirectX::SimpleMath::Vector3 m_recoilRotationDegrees = {};

    WeaponAnimationTuning m_tuning;
    WeaponAnimationOutput m_composed;
};
