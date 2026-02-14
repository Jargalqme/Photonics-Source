#pragma once
#include "Scene.h"
#include "AudioManager.h"
#include "MenuBackground.h"

class SceneManager;

class VictoryScene : public Scene
{
public:
	VictoryScene(SceneManager* sceneManager);
	~VictoryScene() override;

	void Initialize(DX::DeviceResources* deviceResources) override;
	void Enter() override;
	void Exit() override;
	void Cleanup() override;

	void Update(float deltaTime, InputManager* input) override;
	void Render(Renderer* renderer) override;

	void OnDeviceLost() override;
	void OnDeviceRestored() override;

private:
	SceneManager* m_sceneManager;
	std::unique_ptr<AudioManager> m_audioManager;
	std::unique_ptr<MenuBackground> m_background;

	int m_selectedIndex = 0;
	const int m_menuItemCount = 2;
};
