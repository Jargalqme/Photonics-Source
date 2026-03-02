#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>
#include "MeshPart.h"

class EnemyBoss
{
public:
	EnemyBoss(DX::DeviceResources* deviceResources);

	void initialize();

	void update(float deltaTime);

	void render(const Matrix& view, const Matrix& projection);

	void onDeviceLost();

	Vector3 getPosition() const { return m_position; }
	void setPosition(const Vector3& pos) { m_position = pos; }
	const BoundingSphere& getBoundingSphere() const { return m_boundingSphere; }

private:
	DX::DeviceResources* m_deviceResources;

	void buildBoss();

	std::vector<MeshPart> m_parts;

	DirectX::BoundingSphere m_boundingSphere;

	Vector3 m_position;

	Color m_color;
};