#pragma once
#include "Scene.h"
#include "Camera.h"
#include "GridFloor.h"
#include "RoomData.h"
#include <memory>

class SceneManager;

class EditorScene : public Scene
{
public:
	EditorScene(SceneManager* sceneManager);
	~EditorScene() override;

	// Scene interface
	void initialize(DX::DeviceResources* deviceResources) override;
	void enter() override;
	void exit() override;
	void finalize() override;

	void update(float deltaTime, InputManager* input) override;
	void render(Renderer* renderer) override;

private:
	SceneManager* m_sceneManager;

	// Core systems
	std::unique_ptr<Camera> m_camera;

	// World
	std::unique_ptr<GridFloor> m_gridFloor;

	// Room being edited
	RoomData m_roomData;

	// Editor UI
	void renderEditorUI();
};