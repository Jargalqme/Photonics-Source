#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>
#include "MeshPart.h"
#include "Transform.h"

class Core
{
public:

	Core(DX::DeviceResources* deviceResources);

	void Initialize();
	void Update(float deltaTime);
	void Render(const Matrix& view, const Matrix& projection);
	void OnDeviceLost();

	void TakeDamage(float amount);
	bool IsAlive() const { return m_alive; }

	Vector3 GetPosition() const { return m_transform.position; }
	float GetHealth() const { return m_health; }
	float GetMaxHealth() const { return m_maxHealth; }

	void SetPosition(const Vector3& pos);
	void SetColor(const Color& color);
	void SetHealth(float health) { m_health = health; m_maxHealth = health; }
	void Reset(float health);

	const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }


private:

	DX::DeviceResources* m_deviceResources;

	void BuildCore();

	std::vector<MeshPart> m_parts;

	Transform m_transform;

	DirectX::BoundingSphere m_boundingSphere;

	Color m_color;

	float m_health;
	float m_maxHealth;
	bool m_alive;
};