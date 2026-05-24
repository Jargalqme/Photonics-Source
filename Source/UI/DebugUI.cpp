#include "pch.h"
#include "UI/DebugUI.h"
#include "Common/Camera.h"
#include "Gameplay/Player.h"
#include "Gameplay/Weapon/PlayerWeapon.h"
#include "Render/Grid.h"
#include "Render/Skybox.h"
#include "Services/AudioManager.h"
#include "Services/BeatTracker.h"
#include "Gameplay/BulletPool.h"
#include "Gameplay/Boss.h"
#include "Render/Bloom.h"
#include "Services/InputManager.h"
#include <imgui.h>

namespace
{
    void DrawUnavailable(const char* label)
    {
        ImGui::TextDisabled("%s unavailable", label);
    }

    void DrawSection(const char* label)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("%s", label);
    }

    void DrawLabelValue(const char* label, const char* value)
    {
        ImGui::TextDisabled("%s", label);
        ImGui::SameLine(160.0f);
        ImGui::Text("%s", value);
    }
}

void DebugUI::render()
{
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.0f, 700.0f), ImGuiCond_FirstUseEver);

    const bool visible = ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoCollapse);
    if (visible)
    {
        ImGuiIO& io = ImGui::GetIO();
        const float frameMs = io.Framerate > 0.0f ? 1000.0f / io.Framerate : 0.0f;
        ImGui::Text("FPS: %.1f  (%.2f ms)", io.Framerate, frameMs);
        ImGui::TextDisabled("F3: close debug");

        if (m_input)
        {
#ifdef _DEBUG
            DrawLabelValue("Build", "Debug");
#else
            DrawLabelValue("Build", "Release");
#endif
            const char* desired = m_input->isCursorVisible() ? "visible / absolute" : "hidden / relative";
            const char* actual = m_input->isSystemCursorVisible() ? "visible" : "hidden";
            DrawLabelValue("Cursor mode", desired);
            DrawLabelValue("OS cursor", actual);
        }
        else
        {
            DrawUnavailable("Input");
        }

        DrawSection("Camera");
        if (m_camera)
        {
            CameraState* follow = m_camera->getFollowStatePtr();
            ImGui::Text("Follow");
            ImGui::SliderFloat("Follow Height", &follow->height, 0.5f, 10.0f);
            ImGui::SliderFloat("Follow FOV", &follow->fov, 30.0f, 90.0f);

            ImGui::Spacing();
            CameraState* aim = m_camera->getAimStatePtr();
            ImGui::Text("Aim");
            ImGui::SliderFloat("Aim Height", &aim->height, 0.5f, 8.0f);
            ImGui::SliderFloat("Aim FOV", &aim->fov, 20.0f, 70.0f);

            ImGui::Spacing();
            if (m_player)
            {
                ImGui::SliderFloat("Mouse Sens", m_player->getMouseSensitivityPtr(), 0.005f, 0.200f, "%.3f");
            }
            else
            {
                DrawUnavailable("Player mouse tuning");
            }
            ImGui::Checkbox("Rule of Thirds Overlay", &m_showThirds);
        }
        else
        {
            DrawUnavailable("Camera");
        }

        DrawSection("Player");
        if (m_player)
        {
            Vector3 pos = m_player->getPosition();
            ImGui::Text("World Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
            Transform* t = m_player->getTransformPtr();

            ImGui::SliderFloat3("Scale", &t->scale.x, 0.1f, 3.0f);
            ImGui::SliderFloat3("Rotation", &t->rotation.x, -3.14f, 3.14f);
            ImGui::SliderFloat3("Position", &t->position.x, -100.0f, 100.0f);
        }
        else
        {
            DrawUnavailable("Player");
        }

        DrawSection("Weapon");
        if (m_player)
        {
            WeaponMotionTuning* t = m_player->getWeapon().getMotionTuning();

            ImGui::Text("Pose");
            ImGui::SliderFloat3("Hip Position", &t->hipPosition.x, -1.0f, 1.0f);
            ImGui::SliderFloat3("ADS Position", &t->adsPosition.x, -1.0f, 1.0f);
            ImGui::SliderFloat("ADS Blend Speed", &t->adsBlendSpeed, 0.0f, 30.0f);
            ImGui::Spacing();

            ImGui::Text("Sway");
            ImGui::SliderFloat("Sway Max Delta", &t->swayMaxDeltaDeg, 0.0f, 60.0f);
            ImGui::SliderFloat("Sway Position Gain", &t->swayPositionGain, 0.0f, 0.02f);
            ImGui::SliderFloat("Sway Rotation Gain", &t->swayRotationGain, 0.0f, 1.0f);
            ImGui::SliderFloat("Sway Return Speed", &t->swayReturnSpeed, 0.0f, 30.0f);
            ImGui::Spacing();

            ImGui::Text("Movement Bob");
            ImGui::SliderFloat("Bob Frequency", &t->bobFrequency, 0.0f, 20.0f);
            ImGui::SliderFloat("Bob Amplitude X", &t->bobAmplitudeX, 0.0f, 0.1f);
            ImGui::SliderFloat("Bob Amplitude Y", &t->bobAmplitudeY, 0.0f, 0.1f);
            ImGui::SliderFloat("Bob Falloff", &t->bobFalloff, 0.0f, 30.0f);
            ImGui::Spacing();

            ImGui::Text("Recoil");
            ImGui::SliderFloat("Recoil Kickback", &t->recoilKickback, -30.0f, 0.0f);
            ImGui::SliderFloat("Recoil Pitch", &t->recoilPitchDeg, -30.0f, 0.0f);
        }
        else
        {
            DrawUnavailable("Weapon");
        }

        DrawSection("Rendering");
        if (m_exposure)
        {
            ImGui::SliderFloat("Exposure", m_exposure, 0.0f, 3.0f);
        }
        else
        {
            DrawUnavailable("Exposure");
        }

        ImGui::Spacing();
        if (m_grid)
        {
            ImGui::Text("Grid");
            ImGui::SliderFloat("Line Width X", m_grid->getLineWidthXPtr(), 0.001f, 0.1f);
            ImGui::SliderFloat("Line Width Y", m_grid->getLineWidthYPtr(), 0.001f, 0.1f);
            ImGui::SliderFloat("Grid Scale", m_grid->getGridScalePtr(), 0.1f, 5.0f);
            ImGui::SliderFloat("Grid Emissive", m_grid->getLineEmissiveIntensityPtr(), 0.0f, 8.0f);
            ImGui::ColorEdit4("Line Color", m_grid->getLineColorPtr());
            ImGui::ColorEdit4("Base Color", m_grid->getBaseColorPtr());
        }
        else
        {
            DrawUnavailable("Grid");
        }

        ImGui::Spacing();
        if (m_bloom)
        {
            ImGui::Text("Bloom");
            ImGui::Checkbox("Enable", m_bloom->getEnabledPtr());
            ImGui::SliderFloat("Threshold", m_bloom->getThresholdPtr(), 0.0f, 5.0f);
            ImGui::SliderFloat("Knee", m_bloom->getKneePtr(), 0.0f, 1.0f);
            ImGui::SliderFloat("Intensity", m_bloom->getIntensityPtr(), 0.0f, 5.0f);
            ImGui::SliderFloat("Upsample Scale", m_bloom->getUpsampleScalePtr(), 0.25f, 3.0f);
        }
        else
        {
            DrawUnavailable("Bloom");
        }

        DrawSection("Combat");
        if (m_bulletPool)
        {
            Bullet* bullets = m_bulletPool->getBullets();
            const int max = m_bulletPool->getMaxBullets();
            int activeCount = 0;

            for (int i = 0; i < max; i++)
            {
                if (bullets[i].isActive())
                {
                    activeCount++;
                }
            }

            ImGui::Text("Active Bullets: %d / %d", activeCount, max);
            ImGui::ProgressBar(max > 0 ? static_cast<float>(activeCount) / static_cast<float>(max) : 0.0f);

            if (activeCount >= max)
            {
                ImGui::Text("POOL FULL");
            }
        }
        else
        {
            DrawUnavailable("Bullet pool");
        }

        if (m_boss)
        {
            ImGui::Spacing();
            Vector3 pos = m_boss->getPosition();
            ImGui::Text("Boss");
            ImGui::Text("Activated: %s", m_boss->isActivated() ? "YES" : "NO");
            ImGui::Text("Health: %.0f / %.0f (%.0f%%)",
                m_boss->getHealth(), m_boss->getMaxHealth(),
                m_boss->getHealthPercent() * 100.0f);
            ImGui::ProgressBar(m_boss->getHealthPercent(), ImVec2(-1.0f, 0.0f));
            ImGui::Text("Dead: %s", m_boss->isDead() ? "YES" : "NO");
            ImGui::Text("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
        }

        DrawSection("Audio");
        if (m_audioManager)
        {
            if (ImGui::SliderFloat("Master Volume", m_audioManager->getMasterVolumePtr(), 0.0f, 1.0f))
            {
                m_audioManager->setMasterVolume(m_audioManager->getMasterVolume());
            }
        }
        else
        {
            DrawUnavailable("Audio");
        }

        if (m_beatTracker)
        {
            ImGui::Spacing();
            ImGui::Text("Beat: %d", m_beatTracker->getBeat());
            ImGui::ProgressBar(m_beatTracker->getBeatProgress(), ImVec2(-1.0f, 0.0f), "Beat progress");
        }
        else
        {
            ImGui::TextDisabled("No beat tracker in this scene");
        }
    }

    ImGui::End();

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

        float dotRadius = 4.0f;
        draw->AddCircleFilled(ImVec2(x1, y1), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x2, y1), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x1, y2), dotRadius, dotColor);
        draw->AddCircleFilled(ImVec2(x2, y2), dotRadius, dotColor);
    }
}
