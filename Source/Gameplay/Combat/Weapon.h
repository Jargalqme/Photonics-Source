#pragma once
#include <SimpleMath.h>
#include <vector>

using namespace DirectX::SimpleMath;

class WeaponAnimator;
struct ShotIntent;

class Weapon
{
public:
	virtual ~Weapon() = default;

	virtual void initialize();
	virtual void finalize() {}
	void update(
		float deltaTime,
		const Vector3& hitScanOrigin,
		const Vector3& hitScanDirection,
		const Vector3& tracerStart,
		std::vector<ShotIntent>& outIntents);

	void setDependencies(WeaponAnimator* viewmodel);

	void startFire();
	void stopFire();
	bool canFire() const;
	bool isFiring() const { return m_firing; }

	void reload();
	void cancelReload();
	bool isReloading() const { return m_reloading; }

	int getAmmoCount() const { return m_ammo; }
	int getClipSize() const { return m_clipSize; }
	bool outOfAmmo() const { return m_ammo <= 0; }

protected:
	virtual bool shoot(
		const Vector3& hitScanOrigin,
		const Vector3& hitScanDirection,
		const Vector3& tracerStart,
		std::vector<ShotIntent>& outIntents) = 0;

	void recoilImpulse();

	WeaponAnimator* m_viewmodel = nullptr;

	int m_ammo = 0;
	int m_clipSize = 30;
	float m_fireInterval = 0.1f;
	float m_nextShotTime = 0.0f;
	float m_reloadDuration = 2.0f;
	float m_reloadTimer = 0.0f;
	float m_damage = 10.0f;
	float m_maxRange = 200.0f;
	bool m_firing = false;
	bool m_reloading = false;
};
