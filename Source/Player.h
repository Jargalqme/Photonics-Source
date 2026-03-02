#pragma once
#include "DeviceResources.h"
#include "GeometricPrimitive.h"
#include "Effects.h"
#include <DirectXCollision.h>
#include "MeshPart.h"
#include "Transform.h"

class Player
{
public:
	Player(DX::DeviceResources* deviceResources);

	void initialize();
	void render(const Matrix& view, const Matrix& projection);
	void onDeviceLost();

	void moveInDirection(const Vector3& moveDirection, float aimYaw, float deltaTime);
	void jump();
	bool isGrounded() const { return m_isGrounded; }
	void updateIdle(float aimYaw, float deltaTime);

	void takeDamage(float amount);
	bool isDead() const { return m_health <= 0; }

	// boost
	void activateBoost();
	bool isBoosting() const { return m_boosting; }
	float getBoostTimer() const { return m_boostTimer; }
	float getBoostDuration() const { return m_boostDuration; }

	// Getters
	Vector3 getPosition() const { return m_transform.position; }
	Vector3 getForward() const;
	float getHealth() const { return m_health; }
	float getMaxHealth() const { return m_maxHealth; }

	// Debug UI access
	float* getSpeedPtr() { return &m_speed; }
	float* getColorPtr() { return reinterpret_cast<float*>(&m_color); }

	// Return pointers for camera to track
	Vector3* getPositionPtr() { return &m_transform.position; }
	float* getRotationPtr() { return &m_transform.rotation.y; }

	Transform* getTransformPtr() { return &m_transform; }

	void setPosition(const Vector3& pos) { m_transform.position = pos; }
	void setColor(const Color& color) { m_color = color; }
	void setHealth(float health) { m_health = health; }
	void reset();
	const DirectX::BoundingSphere& getBoundingSphere() const { return m_boundingSphere; }

private:
	void buildPlayer();
	DX::DeviceResources* m_deviceResources;

	// Rendering - composite model built from MeshParts
	std::vector<MeshPart> m_parts;
	Color m_color;

	DirectX::BoundingSphere m_boundingSphere;

	Transform m_transform;

	float m_speed;
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