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
	void initialize(DX::DeviceResources* deviceResources) override;
	void enter() override;
	void exit() override;
	void finalize() override;

	void update(float deltaTime, InputManager* input) override;
	void render(Renderer* renderer) override;

private:
	SceneManager* m_sceneManager; // To switch scenes
	std::unique_ptr<AudioManager> m_audioManager;
	std::unique_ptr<MenuBackground> m_background;

	// Menu navigation
	int m_selectedIndex = 0;		// currently selected option
	const int m_menuItemCount = 2;  // total menu options (start, quit)

	void renderMenu();
};
