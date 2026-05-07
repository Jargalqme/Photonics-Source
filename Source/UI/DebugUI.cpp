#include "pch.h"
#include "UI/DebugUI.h"
#include "Common/Camera.h"
#include "Gameplay/Player.h"
#include "Render/Grid.h"
#include "Render/Skybox.h"
#include "Services/AudioManager.h"
#include "Services/BeatTracker.h"
#include "Gameplay/BulletPool.h"
#include "Gameplay/Boss.h"
#include "Render/Bloom.h"
#include "Services/InputManager.h"
#include <imgui.h>

void DebugUI::render()
{
    ImGui::Begin("PHOTON Debug");

    ImGui::StyleColorsClassic();

    // FPS
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    bool cursorVisible = m_input ? m_input->isCursorVisible() : false;
    ImGui::Text("Cursor: %s (Tab to toggle)", cursorVisible ? "VISIBLE" : "HIDDEN");

    // === カメラ ===
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        CameraState* follow = m_camera->getFollowStatePtr();
        ImGui::Text("--- Follow State ---");
        ImGui::SliderFloat("Follow Height", &follow->height, 0.5f, 10.0f);
        ImGui::SliderFloat("Follow FOV", &follow->fov, 30.0f, 90.0f);

        CameraState* aim = m_camera->getAimStatePtr();
        ImGui::Text("--- Aim State ---");
        ImGui::SliderFloat("Aim Height", &aim->height, 0.5f, 8.0f);
        ImGui::SliderFloat("Aim FOV", &aim->fov, 20.0f, 70.0f);

        ImGui::Separator();
        ImGui::SliderFloat("Mouse Sens", m_player->getMouseSensitivityPtr(), 5.0f, 200.0f);
        ImGui::Checkbox("Rule of Thirds", &m_showThirds);
    }

    ImGui::Separator();

    // === プレイヤー ===
    if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Vector3 pos = m_player->getPosition();
        ImGui::Text("World Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);

        if (ImGui::TreeNode("Transform (SRT)"))
        {
            Transform* t = m_player->getTransformPtr();

            ImGui::Text("Scale:    %.2f, %.2f, %.2f", t->scale.x, t->scale.y, t->scale.z);
            ImGui::Text("Rotation: %.2f, %.2f, %.2f (rad)", t->rotation.x, t->rotation.y, t->rotation.z);
            ImGui::Text("Position: %.2f, %.2f, %.2f", t->position.x, t->position.y, t->position.z);

            ImGui::Separator();

            ImGui::SliderFloat3("Scale", &t->scale.x, 0.1f, 3.0f);
            ImGui::SliderFloat3("Rotation", &t->rotation.x, -3.14f, 3.14f);
            ImGui::SliderFloat3("Position", &t->position.x, -100.0f, 100.0f);

            ImGui::TreePop();
        }
        ImGui::Separator();
    }

    // === 武器プロシージャルアニメーション ===
    if (m_player && ImGui::CollapsingHeader("Weapon"))
    {
        WeaponTuning* t = m_player->getViewmodel()->getTuningPtr();

        ImGui::Text("--- Recoil ---");
        ImGui::SliderFloat("Recoil Impulse Z",    &t->recoilImpulseZ,     -20.0f, 0.0f);

        ImGui::Text("--- Aim Punch ---");
        ImGui::SliderFloat("Aim Punch Impulse",   &t->aimPunchImpulseDeg, -20.0f, 0.0f);

        ImGui::Text("--- Bob ---");
        ImGui::SliderFloat("Bob Frequency",       &t->bobFrequency,        0.0f, 20.0f);
        ImGui::SliderFloat("Bob Amplitude X",     &t->bobAmplitudeX,       0.0f,  0.1f);
        ImGui::SliderFloat("Bob Amplitude Y",     &t->bobAmplitudeY,       0.0f,  0.1f);

        ImGui::Text("--- Sway ---");
        ImGui::SliderFloat("Sway Gain",           &t->swayGain,            0.0f,  0.02f);

        ImGui::Text("--- Landing ---");
        ImGui::SliderFloat("Land Gain Y",         &t->landGainY,           0.0f,  0.1f);
        ImGui::SliderFloat("Land Gain Pitch",     &t->landGainPitch,       0.0f, 10.0f);

        ImGui::Separator();
    }

    // === グリッド ===
    if (ImGui::CollapsingHeader("Grid"))
    {
        ImGui::SliderFloat("Line Width X", m_grid->getLineWidthXPtr(), 0.001f, 0.1f);
        ImGui::SliderFloat("Line Width Y", m_grid->getLineWidthYPtr(), 0.001f, 0.1f);
        ImGui::SliderFloat("Grid Scale", m_grid->getGridScalePtr(), 0.1f, 5.0f);
        ImGui::ColorEdit4("Line Color", m_grid->getLineColorPtr());
        ImGui::ColorEdit4("Base Color", m_grid->getBaseColorPtr());
    }

    // === オーディオ ===
    if (ImGui::CollapsingHeader("Audio & Music"))
    {
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

    // === 弾プール ===
    if (ImGui::CollapsingHeader("Bullet Pool", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (m_bulletPool)
        {
            Bullet* bullets = m_bulletPool->getBullets();
            int max = m_bulletPool->getMaxBullets();
            int activeCount = 0;
            int playerCount = 0;
            int bossCount = 0;

            for (int i = 0; i < max; i++)
            {
                if (bullets[i].isActive())
                {
                    activeCount++;
                    if (bullets[i].getFaction() == CombatFaction::Player)
                    {
                        playerCount++;
                    }
                    else
                    {
                        bossCount++;
                    }
                }
            }

            ImGui::Text("Active: %d / %d", activeCount, max);
            ImGui::ProgressBar(static_cast<float>(activeCount) / max);
            ImGui::Text("Player: %d  |  Boss: %d", playerCount, bossCount);

            if (activeCount >= max)
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "POOL FULL!");
            }
        }
    }

    // === ボス ===
    if (ImGui::CollapsingHeader("Boss"))
    {
        if (m_boss)
        {
            ImGui::Text("Activated: %s", m_boss->isActivated() ? "YES" : "NO");
            ImGui::Text("Health: %.0f / %.0f (%.0f%%)",
                m_boss->getHealth(), m_boss->getMaxHealth(),
                m_boss->getHealthPercent() * 100.0f);
            ImGui::Text("Dead: %s", m_boss->isDead() ? "YES" : "NO");
            Vector3 pos = m_boss->getPosition();
            ImGui::Text("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
        }
        else
        {
            ImGui::Text("No boss reference");
        }
    }

    // === ブルーム ===
    if (m_bloom && ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Enable", m_bloom->getEnabledPtr());
        ImGui::SliderFloat("Threshold", m_bloom->getThresholdPtr(), 0.0f, 5.0f);
        ImGui::SliderFloat("Knee", m_bloom->getKneePtr(), 0.0f, 1.0f);
        ImGui::SliderFloat("Intensity", m_bloom->getIntensityPtr(), 0.0f, 5.0f);
        ImGui::SliderFloat("Exposure", m_bloom->getExposurePtr(), 0.0f, 3.0f);
    }

    ImGui::End();

    // === 三分割ガイド ===
    if (m_showThirds)
    {
        ImDrawList* draw = ImGui::GetForegroundDrawList();
        ImVec2 screen = ImGui::GetIO().DisplaySize;

        float x1 = screen.x / 3.0f;
        float x2 = screen.x * 2.0f / 3.0f;
        float y1 = screen.y / 3.0f;
        float y2 = screen.y * 2.0f / 3.0f;

        ImU32 lineColor = IM_COL32(255, 255, 255, 80);
        ImU32 dotColor = IM_COL32(255, 255, 0, 150);
        float thickness = 1.0f;

        draw->AddLine(ImVec2(x1, 0), ImVec2(x1, screen.y), lineColor, thickness);
        draw->AddLine(ImVec2(x2, 0), ImVec2(x2, screen.y), lineColor, thickness);

        draw->AddLine(ImVec2(0, y1), ImVec2(screen.x, y1), lineColor, thickness);
        draw->AddLine(ImVec2(0, y2), ImVec2(screen.x, y2), lineColor, thickness);

        // パワーポイント（視線が自然に集まる4交点）
        float dotRadius = 4.0f;
        draw->AddCircleFilled(ImVec2(x1, y1), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x2, y1), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x1, y2), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x2, y2), dotRadius, dotColor);
    }
}
