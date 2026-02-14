#pragma once
#include "DeviceResources.h"
#include <DirectXCollision.h>
#include "MeshPart.h"

class Tower
{
public:
	Tower(DX::DeviceResources* deviceResources);

	void Initialize();

	void Update(float deltaTime);

	void Render(const Matrix& view, const Matrix& projection);

	void OnDeviceLost();

	Vector3 GetPosition() const { return m_position; }
	void SetPosition(const Vector3& pos) { m_position = pos; }
	const BoundingSphere& GetBoundingSphere() const { return m_boundingSphere; }

private:
	DX::DeviceResources* m_deviceResources;

	void BuildTokyoBigSight();

	std::vector<MeshPart> m_parts;

	DirectX::BoundingSphere m_boundingSphere;

	Vector3 m_position;

	Color m_color;
};