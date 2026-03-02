#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>

enum class EnemyType
{
	Troop_Type_1
};

class Enemy
{
public:
	Enemy(DX::DeviceResources* deviceResources);

	void initialize();
	void update(float deltaTime);
	void render(const Matrix& view, const Matrix& projection);
	void onDeviceLost();

	void spawn(EnemyType type, const Vector3& startPos, const Vector3& targetPos);
	void takeDamage(float amount);
	void deactivate();

	// Getters
	bool isActive() const { return m_active; }
	Vector3 getPosition() const { return m_position; }
	Vector3 getTargetPosition() const { return m_targetPosition; }
	const DirectX::BoundingSphere& getBoundingSphere() const { return m_boundingSphere; }

	// Setters
	void setPosition(const Vector3& pos) { m_position = pos; }
	void setTargetPosition(const Vector3& pos) { m_targetPosition = pos; }

	// Wave movement control
	void setWaveMovement(bool enabled, float amplitude = 3.0f, float frequency = 2.0f);
	void setSpeed(float speed) { m_speed = speed; }
	float getBaseSpeed() const { return m_baseSpeed; }

private:
	DX::DeviceResources* m_deviceResources;
	std::unique_ptr<DirectX::GeometricPrimitive> m_mesh;

	EnemyType m_type;

	Vector3 m_position;
	Vector3 m_targetPosition; // Core position to move toward
	
	DirectX::BoundingSphere m_boundingSphere;
	
	float m_speed;
	float m_health;
	bool m_active;

	Color m_color;

	// Hit feedback
	Color m_originalColor;
	Color m_hitColor = Color(1.0f, 1.0f, 1.0f); // white flash
	float m_hitFlashTimer = 0.0f;

	// Wave movement properties
	bool m_useWaveMovement = false;
	float m_waveAmplitude = 3.0f;    // Side-to-side distance
	float m_waveFrequency = 2.0f;    // Oscillation speed
	float m_wavePhase = 0.0f;        // Random offset per enemy
	float m_elapsedTime = 0.0f;      // Time since spawn for wave calc
	float m_baseSpeed = 8.0f;        // Original speed
};