#pragma once
#include "DeviceResources.h"
#include "GeometricPrimitive.h"
#include "Effects.h"
#include <DirectXCollision.h>
#include "MeshPart.h"
#include "Transform.h"

class LightCycle
{
public:
	LightCycle(DX::DeviceResources* deviceResources);

	void Initialize();
	void Update(float deltaTime);
	void Render(const Matrix& view, const Matrix& projection);
	void OnDeviceLost();

	// Control interface
	void Accelerate(float deltaTime);
	void Brake(float deltaTime);
	void Turn(float direction, float deltaTime);

	void MoveInDirection(const Vector3& moveDirection, const Vector3& facingDirection, float deltaTime);
	void Jump();
	bool IsGrounded() const { return m_isGrounded; }
	void UpdateIdle(float deltaTime);

	void TakeDamage(float amount);
	bool IsDead() const { return m_health <= 0; }

	// boost
	void ActivateBoost();
	bool IsBoosting() const { return m_boosting; }
	float GetBoostTimer() const { return m_boostTimer; }
	float GetBoostDuration() const { return m_boostDuration; }

	// Getters
	Vector3 GetPosition() const { return m_transform.position; }
	Vector3 GetForward() const;
	float GetSpeed() const { return m_speed; }
	float GetHealth() const { return m_health; }
	float GetMaxHealth() const { return m_maxHealth; }

	// Debug UI access
	float* GetMaxSpeedPtr() { return &m_maxSpeed; }
	float* GetAccelerationPtr() { return &m_acceleration; }
	float* GetBrakeForcePtr() { return &m_brakeForce; }
	float* GetTurnRatePtr() { return &m_turnRate; }
	float* GetFrictionPtr() { return &m_friction; }
	float* GetColorPtr() { return reinterpret_cast<float*>(&m_primaryColor); }

	// Return pointers for camera to track
	Vector3* GetPositionPtr() { return &m_transform.position; }
	float* GetRotationPtr() { return &m_transform.rotation.y; }

	Transform* GetTransformPtr() { return &m_transform; }

	void SetPosition(const Vector3& pos) { m_transform.position = pos; }
	void SetColor(const Color& color) { m_primaryColor = color; }
	void SetHealth(float health) { m_health = health; }
	void Reset();
	const DirectX::BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }

private:
	void BuildWeapon();
	DX::DeviceResources* m_deviceResources;

	// Rendering - composite model built from MeshParts
	std::vector<MeshPart> m_parts;
	Color m_primaryColor;	// body/team color
	Color m_carbonColor;	// carbon fiber (dark gray)
	Color m_glowColor;		// hover pods (cyan)

	DirectX::BoundingSphere m_boundingSphere;

	Transform m_transform;

	// Forward physics
	float m_speed;
	float m_maxSpeed;
	float m_acceleration;
	float m_brakeForce;
	float m_turnRate;
	float m_friction;

	float m_arenaRadius;
	float m_health;
	float m_maxHealth;

	// Boost system
	bool m_boosting;
	float m_boostTimer;
	float m_boostDuration;
	float m_boostSpeedMultiplier;

	// Jump system
	bool m_isGrounded;
	float m_verticalVelocity;
	float m_jumpForce;
	float m_gravity;
	float m_groundLevel;
};