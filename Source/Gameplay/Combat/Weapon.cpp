#include "pch.h"
#include "Gameplay/Combat/Weapon.h"
#include "Gameplay/Events/EventBus.h"
#include "Gameplay/Events/EventTypes.h"
#include "Gameplay/Combat/WeaponAnimator.h"

void Weapon::initialize()
{
	m_ammo = m_clipSize;
	m_nextShotTime = 0.0f;
	m_reloadTimer = 0.0f;
	m_firing = false;
	m_reloading = false;
}

void Weapon::setDependencies(WeaponAnimator* viewmodel)
{
	m_viewmodel = viewmodel;
}

void Weapon::update(
	float deltaTime,
	const Vector3& hitScanOrigin,
	const Vector3& hitScanDirection,
	const Vector3& tracerStart,
	std::vector<ShotIntent>& outIntents)
{
	if (m_nextShotTime > 0.0f)
		m_nextShotTime -= deltaTime;

	if (m_reloading)
	{
		m_reloadTimer -= deltaTime;
		if (m_reloadTimer <= 0.0f)
		{
			m_ammo = m_clipSize;
			m_reloading = false;
			m_reloadTimer = 0.0f;
		}
	}

	if (m_firing && canFire())
	{
		if (shoot(hitScanOrigin, hitScanDirection, tracerStart, outIntents))
		{
			m_nextShotTime = m_fireInterval;
			--m_ammo;
			EventBus::publish(WeaponShotEvent{});
		}
	}
}

void Weapon::startFire()
{
	m_firing = true;
}

void Weapon::stopFire()
{
	m_firing = false;
}

bool Weapon::canFire() const
{
	return m_nextShotTime <= 0.0f && m_ammo > 0 && !m_reloading;
}

void Weapon::reload()
{
	if (m_reloading)           return;
	if (m_ammo >= m_clipSize)  return;

	m_reloading = true;
	m_reloadTimer = m_reloadDuration;
}

void Weapon::cancelReload()
{
	m_reloading = false;
	m_reloadTimer = 0.0f;
}

void Weapon::recoilImpulse()
{
	if (m_viewmodel)
		m_viewmodel->onFire();
}
