#include "pch.h"
#include "CollisionSystem.h"

namespace CollisionSystem
{
	bool checkSphereSphere(
		const DirectX::BoundingSphere& a,
		const DirectX::BoundingSphere& b)
	{
		// BoundingSphere already has Intersects() method
		return a.Intersects(b);
	}

	bool checkRaySphere(
		const DirectX::SimpleMath::Vector3& rayStart,
		const DirectX::SimpleMath::Vector3& rayDirection,
		const DirectX::BoundingSphere& sphere,
		float& outDistance)
	{
		// Step 1: Create local variables (L-values)
		DirectX::XMFLOAT3 startFloat(rayStart.x, rayStart.y, rayStart.z);
		DirectX::XMFLOAT3 dirFloat(rayDirection.x, rayDirection.y, rayDirection.z);

		// Step 2: Take address
		DirectX::XMVECTOR origin = DirectX::XMLoadFloat3(&startFloat);
		DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&dirFloat);

		// BoundingSphere has Intersects() for rays too
		return sphere.Intersects(origin, direction, outDistance);
	}
}
