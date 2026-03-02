#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>


class Bullet
{
public:
	Bullet(DX::DeviceResources* deviceResources);

	void initialize();
	void update(float deltaTime);
	void render(const Matrix& view, const Matrix& projection);
	void onDeviceLost();

	void fire(const Vector3& from, const Vector3& target);
	void deactivate() { m_active = false; }
	bool isActive() const { return m_active; }
	Vector3 getPosition() const { return m_position; }
	const DirectX::BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }

private:
	DX::DeviceResources* m_deviceResources;
	std::unique_ptr<DirectX::GeometricPrimitive> m_mesh;

	DirectX::BoundingSphere m_boundingSphere;


	Vector3 m_position;
	Vector3 m_direction;
	float m_speed;
	bool m_active;

	Color m_color;
};