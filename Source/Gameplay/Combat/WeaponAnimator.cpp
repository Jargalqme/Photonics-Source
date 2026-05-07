#include "pch.h"
#include "Gameplay/Combat/WeaponAnimator.h"

using namespace DirectX::SimpleMath;



void WeaponAnimator::update(float dt)
{
	WeaponOffset recoil   = computeRecoil(dt);
	WeaponOffset aimPunch = computeAimPunch(dt);
	WeaponOffset bob      = computeBob(dt);
	WeaponOffset sway     = computeSway(dt);
	WeaponOffset land     = computeLanding(dt);

	m_composed.pos    = recoil.pos    + aimPunch.pos    + bob.pos    + sway.pos    + land.pos;
	m_composed.rotDeg = recoil.rotDeg + aimPunch.rotDeg + bob.rotDeg + sway.rotDeg + land.rotDeg;
}



void WeaponAnimator::onFire()
{
	m_recoilZ.kick(m_tuning.recoilImpulseZ);
	m_aimPitch.kick(m_tuning.aimPunchImpulseDeg);
}



void WeaponAnimator::reset()
{
	m_recoilZ.reset();
	m_aimPitch.reset();
	m_bobPhase  = 0.0f;
	m_bobAmount = 0.0f;
	m_swayX.reset();
	m_swayY.reset();
	m_landY.reset();
	m_landPitch.reset();
	m_composed = {};
}



void WeaponAnimator::onLand(float fallSpeed)
{
	if (fallSpeed < m_landMinSpeed) return;                // 小さな跳ねは無視
	fallSpeed = std::min(fallSpeed, m_landMaxSpeed);
	m_landY.kick(-fallSpeed * m_tuning.landGainY);         // 武器を下へ沈める
	m_landPitch.kick(fallSpeed * m_tuning.landGainPitch);  // 視点を下へ
}



void WeaponAnimator::addLookDelta(float yawDeg, float pitchDeg)
{
	yawDeg   = std::clamp(yawDeg,   -m_swayMaxKick, m_swayMaxKick);
	pitchDeg = std::clamp(pitchDeg, -m_swayMaxKick, m_swayMaxKick);
	m_swayX.kick(-yawDeg   * m_tuning.swayGain);   // 視点が右なら武器は左へ遅れる
	m_swayY.kick(-pitchDeg * m_tuning.swayGain);
}



WeaponOffset WeaponAnimator::computeRecoil(float dt)
{
	m_recoilZ.update(0.0f, dt);
	return { Vector3(0.0f, 0.0f, m_recoilZ.x), Vector3::Zero };
}



WeaponOffset WeaponAnimator::computeAimPunch(float dt)
{
	m_aimPitch.update(0.0f, dt);
	return { Vector3::Zero, Vector3(m_aimPitch.x, 0.0f, 0.0f) };
}



WeaponOffset WeaponAnimator::computeBob(float dt)
{
	constexpr float kMaxSpeed = 15.0f;   // Player::m_speed と一致

	// 移動・接地時のみアクティブ
	float speedN = std::clamp(m_game.moveSpeed / kMaxSpeed, 0.0f, 1.0f);
	float target = (m_game.grounded && speedN > 0.05f) ? speedN : 0.0f;
	m_bobAmount += (target - m_bobAmount) * std::min(1.0f, m_bobFalloff * dt);

	// フェーズは実効速度でのみ進める（停止中は凍結）
	m_bobPhase += m_tuning.bobFrequency * speedN * dt;

	float x = std::sin(m_bobPhase)        * m_tuning.bobAmplitudeX * m_bobAmount;
	float y = std::cos(m_bobPhase * 2.0f) * m_tuning.bobAmplitudeY * m_bobAmount;

	return { Vector3(x, y, 0.0f), Vector3::Zero };
}



WeaponOffset WeaponAnimator::computeSway(float dt)
{
	m_swayX.update(0.0f, dt);
	m_swayY.update(0.0f, dt);
	return { Vector3(m_swayX.x, m_swayY.x, 0.0f), Vector3::Zero };
}



WeaponOffset WeaponAnimator::computeLanding(float dt)
{
	m_landY.update(0.0f, dt);
	m_landPitch.update(0.0f, dt);
	return { Vector3(0.0f, m_landY.x, 0.0f), Vector3(m_landPitch.x, 0.0f, 0.0f) };
}
