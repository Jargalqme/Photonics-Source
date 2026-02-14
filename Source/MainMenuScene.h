#pragma once
#include "Scene.h"
#include "AudioManager.h"
#include "MenuBackground.h"

class SceneManager;

class MainMenuScene : public Scene
{
public:
	MainMenuScene(SceneManager* sceneManager);
	~MainMenuScene() override;

	// Scene interface
	void Initialize(DX::DeviceResources* deviceResources) override;
	void Enter() override;
	void Exit() override;
	void Cleanup() override;

	void Update(float deltaTime, InputManager* input) override;
	void Render(Renderer* renderer) override;

	void OnDeviceLost() override;
	void OnDeviceRestored() override;

private:
	SceneManager* m_sceneManager; // To switch scenes
	std::unique_ptr<AudioManager> m_audioManager;
	std::unique_ptr<MenuBackground> m_background;

	// Menu navigation
	int m_selectedIndex = 0;		// currently selected option
	const int m_menuItemCount = 2;  // total menu options (start, quit)

	void RenderMenu();
};
