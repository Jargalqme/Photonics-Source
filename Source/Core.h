#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>
#include "MeshPart.h"
#include "Transform.h"

class Core
{
public:

	Core(DX::DeviceResources* deviceResources);

	void initialize();
	void update(float deltaTime);
	void render(const Matrix& view, const Matrix& projection);
	void onDeviceLost();

	void takeDamage(float amount);
	bool isAlive() const { return m_alive; }

	Vector3 getPosition() const { return m_transform.position; }
	float getHealth() const { return m_health; }
	float getMaxHealth() const { return m_maxHealth; }

	void setPosition(const Vector3& pos);
	void setColor(const Color& color);
	void setHealth(float health) { m_health = health; m_maxHealth = health; }
	void reset(float health);

	const BoundingSphere& getBoundingSphere() const { return m_boundingSphere; }


private:

	DX::DeviceResources* m_deviceResources;

	void buildCore();

	std::vector<MeshPart> m_parts;

	Transform m_transform;

	DirectX::BoundingSphere m_boundingSphere;

	Color m_color;

	float m_health;
	float m_maxHealth;
	bool m_alive;
};