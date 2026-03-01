#pragma once
#include <DirectXCollision.h>

namespace CollisionSystem
{
	// Sphere vs Sphere
	// Returns true if spheres overlap
	bool CheckSphereSphere(
		const DirectX::BoundingSphere& a,
		const DirectX::BoundingSphere& b);

	// Ray vs Sphere
	// Returns true if ray hits sphere
	// outDistance = how far along ray the hit occurred
	bool CheckRaySphere(
		const DirectX::SimpleMath::Vector3& rayStart,
		const DirectX::SimpleMath::Vector3& rayDirection,
		const DirectX::BoundingSphere& sphere,
		float& outDistance);
}
