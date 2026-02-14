#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>


class Bullet
{
public:
	Bullet(DX::DeviceResources* deviceResources);

	void Initialize();
	void Update(float deltaTime);
	void Render(const Matrix& view, const Matrix& projection);
	void OnDeviceLost();

	void Fire(const Vector3& from, const Vector3& target);
	void Deactivate() { m_active = false; }
	bool IsActive() const { return m_active; }
	Vector3 GetPosition() const { return m_position; }
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