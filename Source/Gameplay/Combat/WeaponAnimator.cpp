#include "pch.h"
#include "Gameplay/Combat/WeaponAnimator.h"

using namespace DirectX::SimpleMath;

namespace
{
    float clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    float normalizedLerpStep(float speed, float deltaTime)
    {
        return clamp01(speed * std::max(deltaTime, 0.0f));
    }

    float lerpFloat(float from, float to, float amount)
    {
        return from + (to - from) * amount;
    }

    Vector3 lerpVector(const Vector3& from, const Vector3& to, float amount)
    {
        return from + (to - from) * amount;
    }

    void addLayer(WeaponAnimationOutput& total, const WeaponAnimationOutput& layer)
    {
        total.position += layer.position;
        total.rotationDegrees += layer.rotationDegrees;
    }
}

void WeaponAnimator::update(const WeaponAnimationInput& input)
{
    const float deltaTime = std::max(input.deltaTime, 0.0f);

    updateAds(deltaTime, input.isAiming);

    m_composed = {};
    addLayer(m_composed, computeBasePose());

    if (m_tuning.enableSway)
    {
        addLayer(m_composed, computeSway(input.lookDeltaDegrees, deltaTime));
    }
    else
    {
        m_swayPosition = {};
        m_swayRotationDegrees = {};
    }

    if (m_tuning.enableBob)
    {
        addLayer(m_composed, computeBob(input.moveSpeed01, input.isGrounded, deltaTime));
    }
    else
    {
        m_bobAmount = 0.0f;
    }

    if (m_tuning.enableRecoil)
    {
        addLayer(m_composed, computeRecoil(deltaTime));
    }
    else
    {
        m_recoilPosition = {};
        m_recoilRotationDegrees = {};
    }
}

void WeaponAnimator::onWeaponFired()
{
    if (!m_tuning.enableRecoil)
    {
        return;
    }

    // Recoil is visual-only. The weapon moves back and pitches up, but gameplay
    // hits still use the camera ray owned by PlayerSystem.
    m_recoilPosition.z = std::clamp(
        m_recoilPosition.z - m_tuning.recoilKickback,
        -m_tuning.recoilMaxKickback,
        0.0f);

    m_recoilRotationDegrees.x = std::clamp(
        m_recoilRotationDegrees.x - m_tuning.recoilPitchDegrees,
        -m_tuning.recoilMaxPitchDegrees,
        m_tuning.recoilMaxPitchDegrees);
}

void WeaponAnimator::reset()
{
    m_adsAlpha = 0.0f;
    m_swayPosition = {};
    m_swayRotationDegrees = {};
    m_bobPhase = 0.0f;
    m_bobAmount = 0.0f;
    m_recoilPosition = {};
    m_recoilRotationDegrees = {};
    m_composed = {};
}

void WeaponAnimator::updateAds(float deltaTime, bool isAiming)
{
    const float target = (m_tuning.enableAds && isAiming) ? 1.0f : 0.0f;
    const float step = normalizedLerpStep(m_tuning.adsBlendSpeed, deltaTime);
    m_adsAlpha = lerpFloat(m_adsAlpha, target, step);
}

WeaponAnimationOutput WeaponAnimator::computeBasePose() const
{
    // The base pose is the weapon's rest position. ADS is just a readable blend
    // between the hip pose and the aimed pose; no procedural layer should know
    // where those authored poses live.
    WeaponAnimationOutput output;
    output.position = lerpVector(m_tuning.hipPosition, m_tuning.adsPosition, m_adsAlpha);
    output.rotationDegrees = lerpVector(
        m_tuning.hipRotationDegrees,
        m_tuning.adsRotationDegrees,
        m_adsAlpha);
    return output;
}

WeaponAnimationOutput WeaponAnimator::computeSway(const Vector2& lookDeltaDegrees, float deltaTime)
{
    const float maxLookDelta = std::max(m_tuning.swayMaxLookDeltaDegrees, 0.0f);
    const float yawDelta = std::clamp(lookDeltaDegrees.x, -maxLookDelta, maxLookDelta);
    const float pitchDelta = std::clamp(lookDeltaDegrees.y, -maxLookDelta, maxLookDelta);

    // Sway is weapon lag. When the camera turns right, the weapon trails left.
    // The target is small and directly proportional to this frame's look delta.
    const Vector3 targetPosition(
        -yawDelta * m_tuning.swayPositionAmount,
        -pitchDelta * m_tuning.swayPositionAmount,
        0.0f);

    const Vector3 targetRotationDegrees(
        -pitchDelta * m_tuning.swayRotationAmountDegrees,
        -yawDelta * m_tuning.swayRotationAmountDegrees,
        0.0f);

    const float step = normalizedLerpStep(m_tuning.swayReturnSpeed, deltaTime);
    m_swayPosition = lerpVector(m_swayPosition, targetPosition, step);
    m_swayRotationDegrees = lerpVector(m_swayRotationDegrees, targetRotationDegrees, step);

    return { m_swayPosition, m_swayRotationDegrees };
}

WeaponAnimationOutput WeaponAnimator::computeBob(float moveSpeed01, bool isGrounded, float deltaTime)
{
    const float speed01 = clamp01(moveSpeed01);
    const float targetAmount = (isGrounded && speed01 > 0.0f) ? speed01 : 0.0f;

    // Bob fades in and out instead of popping when movement starts/stops.
    const float blendStep = normalizedLerpStep(m_tuning.bobBlendSpeed, deltaTime);
    m_bobAmount = lerpFloat(m_bobAmount, targetAmount, blendStep);

    // Phase only advances from movement speed, so an idle weapon settles instead
    // of crawling through the walk cycle while standing still.
    m_bobPhase += m_tuning.bobSpeed * speed01 * deltaTime;

    const float horizontal = std::sin(m_bobPhase);
    const float vertical = std::sin(m_bobPhase * 2.0f);

    WeaponAnimationOutput output;
    output.position = Vector3(
        horizontal * m_tuning.bobHorizontalAmount * m_bobAmount,
        vertical * m_tuning.bobVerticalAmount * m_bobAmount,
        0.0f);
    output.rotationDegrees = Vector3(
        0.0f,
        0.0f,
        horizontal * m_tuning.bobRollDegrees * m_bobAmount);
    return output;
}

WeaponAnimationOutput WeaponAnimator::computeRecoil(float deltaTime)
{
    // Simple recovery: each frame pulls the current recoil offset back to zero.
    // This is intentionally not a physical spring; the baseline needs obvious,
    // predictable behavior before we add more nuance.
    const float step = normalizedLerpStep(m_tuning.recoilReturnSpeed, deltaTime);
    m_recoilPosition = lerpVector(m_recoilPosition, Vector3::Zero, step);
    m_recoilRotationDegrees = lerpVector(m_recoilRotationDegrees, Vector3::Zero, step);

    return { m_recoilPosition, m_recoilRotationDegrees };
}
