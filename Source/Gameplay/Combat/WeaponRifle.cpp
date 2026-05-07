#include "pch.h"
#include "Gameplay/Combat/WeaponRifle.h"
#include "Gameplay/Combat/ShotIntent.h"

void WeaponRifle::initialize()
{
	m_clipSize = 30;
	m_fireInterval = 0.08f;
	m_damage = 12.0f;
	m_maxRange = 150.0f;
	m_reloadDuration = 1.5f;
	Weapon::initialize();
}

bool WeaponRifle::shoot(
	const Vector3& hitScanOrigin,
	const Vector3& hitScanDirection,
	const Vector3& tracerStart,
	std::vector<ShotIntent>& outIntents)
{
	// 武器側はインテントを発行するだけ — ヒット判定・ダメージ・トレーサーは CombatSystem に委譲
	outIntents.push_back(ShotIntent{
		hitScanOrigin,
		hitScanDirection,
		tracerStart,
		m_damage,
		m_maxRange,
		Vector4(0.4f, 0.85f, 1.0f, 1.0f),  // ライフルのトレーサー色（シアン）
		CombatFaction::Player,
	});

	// リコイル（ビューモデルアニメ — 武器自身の責務）
	recoilImpulse();

	return true;
}
