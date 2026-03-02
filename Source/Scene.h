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
	virtual void initialize(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }
	virtual void enter() { m_isActive = true; }
	virtual void exit() { m_isActive = false; }
	virtual void finalize() {}

	// Update and render
	virtual void update(float dt, InputManager* input) = 0;
	virtual void render(Renderer* renderer) = 0;

	// Getters
	const std::string& getName() const { return m_name; }
	bool isActive() const { return m_isActive; }

protected:
	std::string m_name;
	bool m_isActive;
	DX::DeviceResources* m_deviceResources;
};
