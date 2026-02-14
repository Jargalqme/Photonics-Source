#pragma once

#include <string>

// Forward declarations
class InputManager;
class Renderer;
class Camera;

namespace DX {
	class DeviceResources;
}

// Base class for all scenes
class Scene {
public:
	Scene(const std::string& name)
		: m_name(name)
		, m_isActive(false)
		, m_deviceResources(nullptr) 
	{
	}
	virtual ~Scene() = default;

	// Lifecycle methods
	virtual void Initialize(DX::DeviceResources* deviceResources) = 0;
	virtual void Enter() { m_isActive = true; } // Called when scene becomes active
	virtual void Exit() { m_isActive = false; } // Called when leaving scene
	virtual void Cleanup() = 0;					// Called before scene destruction

	// Update and render
	virtual void Update(float deltaTime, InputManager* input) = 0;
	virtual void Render(Renderer* renderer) = 0;

	// Window events (optional overrides)
	virtual void OnWindowSizeChanged(int width, int height) {
		UNREFERENCED_PARAMETER(width);
		UNREFERENCED_PARAMETER(height);
	}
	virtual void OnDeviceLost() {}
	virtual void OnDeviceRestored() {}

	// Getters
	const std::string& GetName() const { return m_name; }
	bool IsActive() const { return m_isActive; }

protected:
	std::string m_name;
	bool m_isActive;
	DX::DeviceResources* m_deviceResources;
};
