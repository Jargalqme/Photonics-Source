//=============================================================================
// @brief    射撃インテント — Weapon が発行、CombatSystem が解決する受動データ
//=============================================================================
#pragma once

#include "Gameplay/Combat/ICombatTarget.h"
#include <SimpleMath.h>

struct ShotIntent
{
	DirectX::SimpleMath::Vector3 hitScanOrigin;
	DirectX::SimpleMath::Vector3 hitScanDirection;
	DirectX::SimpleMath::Vector3 tracerStart;
	float                        damage;
	float                        maxRange;
	DirectX::SimpleMath::Vector4 tracerColor;
	CombatFaction                faction = CombatFaction::Player;
};
