#pragma once
#include "Gameplay/Combat/Weapon.h"

class WeaponRifle : public Weapon
{
public:
	void initialize() override;

protected:
	bool shoot(
		const Vector3& hitScanOrigin,
		const Vector3& hitScanDirection,
		const Vector3& tracerStart,
		std::vector<ShotIntent>& outIntents) override;
};
