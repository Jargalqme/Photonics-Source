#include "pch.h"
#include "DebugUI.h"
#include "Camera.h"
#include "Player.h"
#include "GridFloor.h"
#include "Terrain.h"
#include "ProjectilePool.h"
#include "AudioManager.h"
#include "MusicManager.h"
#include "InputManager.h"
#include <imgui.h>

void DebugUI::render()
{
    ImGui::Begin("PHOTON Debug");

	// FPS
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Separator();

	bool cursorVisible = m_input ? m_input->isCursorVisible() : false;
	ImGui::Text("Cursor: %s (Tab to toggle)", cursorVisible ? "VISIBLE" : "HIDDEN");

	// Camera Mode
	ImGui::Text("Camera: %s (F1/F2 to switch)", m_camera->getModeName());
	ImGui::Separator();

	// LightCycle
	if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Position info
		Vector3 pos = m_player->getPosition();
		ImGui::Text("World Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);

		if (ImGui::TreeNode("Transform (SRT)"))
		{
			Transform* t = m_player->getTransformPtr();

			ImGui::Text("Scale:    %.2f, %.2f, %.2f", t->scale.x, t->scale.y, t->scale.z);
			ImGui::Text("Rotation: %.2f, %.2f, %.2f (rad)", t->rotation.x, t->rotation.y, t->rotation.z);
			ImGui::Text("Position: %.2f, %.2f, %.2f", t->position.x, t->position.y, t->position.z);

			ImGui::Separator();

			// Editable sliders
			ImGui::SliderFloat3("Scale", &t->scale.x, 0.1f, 3.0f);
			ImGui::SliderFloat3("Rotation", &t->rotation.x, -3.14f, 3.14f);
			ImGui::SliderFloat3("Position", &t->position.x, -100.0f, 100.0f);

			ImGui::TreePop();
		}
		ImGui::Separator();
	}

	// Grid Floor
	if (ImGui::CollapsingHeader("Grid Floor"))
	{
		ImGui::SliderFloat("Line Width X", m_gridFloor->getLineWidthXPtr(), 0.001f, 0.1f);
		ImGui::SliderFloat("Line Width Y", m_gridFloor->getLineWidthYPtr(), 0.001f, 0.1f);
		ImGui::SliderFloat("Grid Scale", m_gridFloor->getGridScalePtr(), 0.1f, 5.0f);
		ImGui::ColorEdit4("Line Color", m_gridFloor->getLineColorPtr());
		ImGui::ColorEdit4("Base Color", m_gridFloor->getBaseColorPtr());
	}

	// Audio & Music
	if (ImGui::CollapsingHeader("Audio & Music"))
	{
		// Audio Manager
		if (m_audioManager)
		{
			ImGui::Text("=== Audio ===");
			if (ImGui::SliderFloat("Master Volume", m_audioManager->getMasterVolumePtr(), 0.0f, 1.0f))
			{
				m_audioManager->setMasterVolume(m_audioManager->getMasterVolume());
			}
			ImGui::Separator();
		}
	}
    ImGui::End();
}