#include "pch.h"
#include "EditorScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "Renderer.h"

EditorScene::EditorScene(SceneManager* sceneManager)
	: Scene("Editor")
	, m_sceneManager(sceneManager)
{
}

EditorScene::~EditorScene()
{
}

void EditorScene::initialize(DX::DeviceResources* deviceResources)
{
	Scene::initialize(deviceResources);

	// Camera - free mode for editor navigation
	m_camera = std::make_unique<Camera>();
	m_camera->setMode(CameraMode::Free);
	m_camera->setPosition(Vector3(0.0f, 30.0f, -50.0f));
	m_camera->setMoveSpeed(40.0f);

	auto output = m_deviceResources->GetOutputSize();
	m_camera->setProjectionParameters(
		45.0f,
		float(output.right) / float(output.bottom),
		0.1f,
		1000.0f
	);

	// Grid floor
	m_gridFloor = std::make_unique<GridFloor>(m_deviceResources);
	m_gridFloor->initialize();
}

void EditorScene::enter()
{
	Scene::enter();
}

void EditorScene::exit()
{
	Scene::exit();
}

void EditorScene::finalize()
{
	if (m_gridFloor) m_gridFloor->finalize();
}

void EditorScene::update(float deltaTime, InputManager* input)
{
	// Escape - back to main menu
	if (input->isKeyPressed(Keyboard::Keys::Escape))
	{
		m_sceneManager->transitionTo("MainMenu");
		return;
	}

	// Camera movement
	m_camera->update(deltaTime, input);
}

void EditorScene::render(Renderer* renderer)
{
	(void)renderer;

	auto view = m_camera->getViewMatrix();
	auto proj = m_camera->getProjectionMatrix();

	// Grid floor
	m_gridFloor->render(view, proj);

	// Editor UI
	renderEditorUI();
}


void EditorScene::renderEditorUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(280, 400), ImGuiCond_FirstUseEver);

	ImGui::Begin("Room Editor");

	// FPS
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Separator();

	// Room settings
	if (ImGui::CollapsingHeader("Room Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		char nameBuf[128];
		strncpy_s(nameBuf, m_roomData.name.c_str(), sizeof(nameBuf) - 1);
		if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
			m_roomData.name = nameBuf;

		ImGui::SliderFloat("Floor Width", &m_roomData.floorWidth, 50.0f, 500.0f);
		ImGui::SliderFloat("Floor Depth", &m_roomData.floorDepth, 50.0f, 500.0f);

		ImGui::DragFloat3("Player Start", &m_roomData.playerStart.x, 0.5f);
		ImGui::DragFloat3("Entry Gate", &m_roomData.entryGatePos.x, 0.5f);
		ImGui::DragFloat3("Exit Gate", &m_roomData.exitGatePos.x, 0.5f);
	}

	ImGui::Separator();

	// Object count
	ImGui::Text("Objects: %d", (int)m_roomData.objects.size());
	ImGui::Text("Spawns: %d", (int)m_roomData.spawns.size());

	ImGui::Separator();

	// Save / Load
	if (ImGui::Button("Save Room", ImVec2(-1, 0)))
	{
		m_roomData.save("Assets/Rooms/" + m_roomData.name + ".json");
	}

	if (ImGui::Button("Load Room", ImVec2(-1, 0)))
	{
		m_roomData.load("Assets/Rooms/" + m_roomData.name + ".json");
	}

	ImGui::Separator();

	// Test play
	if (ImGui::Button("Test Play", ImVec2(-1, 0)))
	{
		m_roomData.save("Assets/Rooms/_testplay.json");
		m_sceneManager->transitionTo("GameScene");
	}

	ImGui::End();
}
