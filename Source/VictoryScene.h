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

	void initialize(DX::DeviceResources* deviceResources) override;
	void enter() override;
	void exit() override;
	void finalize() override;

	void update(float deltaTime, InputManager* input) override;
	void render(Renderer* renderer) override;

private:
	SceneManager* m_sceneManager;
	std::unique_ptr<AudioManager> m_audioManager;
	std::unique_ptr<MenuBackground> m_background;

	int m_selectedIndex = 0;
	const int m_menuItemCount = 2;
};
