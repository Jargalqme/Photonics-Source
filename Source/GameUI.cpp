#include "pch.h"
#include "GameUI.h"
#include "MusicManager.h"
#include "Player.h"
#include "Core.h"
#include "EnemyTroops.h"
#include "BeamWeapon.h"
#include <imgui.h>

GameUI::GameUI()
{
}

void GameUI::initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;
}

void GameUI::setCores(Core* r, Core* g, Core* b)
{
	m_coreRed = r;
	m_coreGreen = g;
	m_coreBlue = b;
}

void GameUI::update(float deltaTime)
{
	// Fade out edge glow over time
	if (m_edgeGlowAlpha > 0.0f)
	{
		m_edgeGlowAlpha -= m_edgeGlowDecay * deltaTime;
		if (m_edgeGlowAlpha < 0.0f)
			m_edgeGlowAlpha = 0.0f;
	}
    // testing
    // Fade out beat pulse
    if (m_beatPulseAlpha > 0.0f)
    {
        m_beatPulseAlpha -= 8.0f * deltaTime; // fast decay
        if (m_beatPulseAlpha < 0.0f)
            m_beatPulseAlpha = 0.0f;
    }
}

void GameUI::render()
{
	//drawBeatBar();
	drawMinimap();
	drawHealthBars();
    //drawBoostIndicator();
    drawSongProgress();
    drawCrosshair();
}

void GameUI::drawBeatBar()
{
    if (!m_musicManager) return;

    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);
    float screenHeight = static_cast<float>(size.bottom - size.top);

    // Bar dimensions
    float barWidth = screenWidth;           // full screen width
    float barHeight = 80.0f;                // Height 80
    float barX = 0.0f;                      // Start from left edge
    float barY = screenHeight - barHeight;  // Anchor to bottom

    // Beat marker dimensions
    float markerWidth = 16.0f;
    float markerHeight = barHeight - 16.0f;

    // Center hit zone
    float centerX = barX + barWidth / 2.0f;
    float hitZoneWidth = 20.0f;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Draw bar background
    drawList->AddRectFilled(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        IM_COL32(20, 20, 20, 220)
    );

    // Draw hit zone (center target area)
    drawList->AddRectFilled(
        ImVec2(centerX - hitZoneWidth / 2, barY),
        ImVec2(centerX + hitZoneWidth / 2, barY + barHeight),
        IM_COL32(255, 255, 255, 40)
    );

    // Center line
    drawList->AddRectFilled(
        ImVec2(centerX - 2, barY),
        ImVec2(centerX + 2, barY + barHeight),
        IM_COL32(255, 255, 255, 150)
    );

    // Get beat progress (0.0 to 1.0)
    float progress = m_musicManager->getBeatProgress();
    int currentBeat = m_musicManager->getBeat();

    // Markers come from both sides toward center
    float spacing = barWidth / 2.0f / 4.0f;  // Space for 6 beats per side

    for (int i = 0; i < 4; i++)
    {
        // Calculate position: markers slide inward
        // Beat 0 (current) is closest to center, beat 3 is at edge
        float beatOffset = (i + (1.0f - progress)) * spacing;

        // Determine color (alternate cyan/magenta)
        int futureBeat = (currentBeat + i) % 4;
        ImU32 color = (futureBeat % 2 == 1)
            ? IM_COL32(0, 255, 255, 255)   // Cyan (odd)
            : IM_COL32(255, 0, 255, 255);  // Magenta (even)

        // Fade out markers further from center
        float alpha = 1.0f - (i * 0.2f);
        int r = (color >> 0) & 0xFF;
        int g = (color >> 8) & 0xFF;
        int b = (color >> 16) & 0xFF;
        color = IM_COL32(r, g, b, (int)(alpha * 255));

        // Left side marker (coming from left)
        float leftX = centerX - beatOffset;
        if (leftX > barX)
        {
            drawList->AddRectFilled(
                ImVec2(leftX - markerWidth / 2, barY + 4),
                ImVec2(leftX + markerWidth / 2, barY + 4 + markerHeight),
                color
            );
        }

        // Right side marker (coming from right)
        float rightX = centerX + beatOffset;
        if (rightX < barX + barWidth)
        {
            drawList->AddRectFilled(
                ImVec2(rightX - markerWidth / 2, barY + 4),
                ImVec2(rightX + markerWidth / 2, barY + 4 + markerHeight),
                color
            );
        }
    }

    // Beat pulse at center when beat hits
    if (m_beatPulseAlpha > 0.0f)
    {
        float pulseRadius = 20.0f + (1.0f - m_beatPulseAlpha) * 15.0f;

        int r = (m_beatColor >> 0) & 0xFF;
        int g = (m_beatColor >> 8) & 0xFF;
        int b = (m_beatColor >> 16) & 0xFF;
        ImU32 pulseColor = IM_COL32(r, g, b, (int)(m_beatPulseAlpha * 200));

        drawList->AddCircleFilled(
            ImVec2(centerX, barY + barHeight / 2.0f),
            pulseRadius,
            pulseColor
        );
    }

    // Border
    drawList->AddRect(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        IM_COL32(100, 100, 100, 255)
    );
}

void GameUI::drawMinimap()
{
    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);

    // Minimap position and size (circle)
    float mapRadius = 120.0f;  // Radius of the circle
    float mapCenterX = screenWidth - mapRadius - 30.0f;  // Right side
    float mapCenterY = mapRadius + 30.0f;                // Top with padding
    float worldSize = 100.0f;  // World goes from -100 to +100

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Draw circular background
    drawList->AddCircleFilled(
        ImVec2(mapCenterX, mapCenterY),
        mapRadius,
        IM_COL32(20, 20, 20, 200),
        32  // Segments for smooth circle
    );

    // Draw circular border
    drawList->AddCircle(
        ImVec2(mapCenterX, mapCenterY),
        mapRadius,
        IM_COL32(100, 100, 100, 255),
        32,
        2.0f  // Border thickness
    );

    // Helper: Convert world position to minimap position
    auto worldToMinimap = [&](Vector3 worldPos) -> ImVec2 {
        float x = (worldPos.x / worldSize) * mapRadius + mapCenterX;
        float y = (-worldPos.z / worldSize) * mapRadius + mapCenterY;  // Z flipped
        return ImVec2(x, y);
        };

    // Helper: Check if point is inside circle
    auto isInsideCircle = [&](ImVec2 pos) -> bool {
        float dx = pos.x - mapCenterX;
        float dy = pos.y - mapCenterY;
        return (dx * dx + dy * dy) <= (mapRadius * mapRadius);
        };

    // Draw Tower (center, white square)
    drawList->AddRectFilled(
        ImVec2(mapCenterX - 4, mapCenterY - 4),
        ImVec2(mapCenterX + 4, mapCenterY + 4),
        IM_COL32(255, 255, 255, 255)
    );

    // Draw Cores (only if inside circle)
    if (m_coreRed && m_coreRed->isAlive())
    {
        ImVec2 pos = worldToMinimap(m_coreRed->getPosition());
        if (isInsideCircle(pos))
            drawList->AddCircleFilled(pos, 5.0f, IM_COL32(255, 0, 0, 255));
    }
    if (m_coreGreen && m_coreGreen->isAlive())
    {
        ImVec2 pos = worldToMinimap(m_coreGreen->getPosition());
        if (isInsideCircle(pos))
            drawList->AddCircleFilled(pos, 5.0f, IM_COL32(0, 255, 0, 255));
    }
    if (m_coreBlue && m_coreBlue->isAlive())
    {
        ImVec2 pos = worldToMinimap(m_coreBlue->getPosition());
        if (isInsideCircle(pos))
            drawList->AddCircleFilled(pos, 5.0f, IM_COL32(0, 100, 255, 255));
    }

    // Draw Enemies (small yellow dots)
    if (m_enemies)
    {
        for (auto& enemy : *m_enemies)
        {
            if (enemy->isActive())
            {
                ImVec2 pos = worldToMinimap(enemy->getPosition());
                if (isInsideCircle(pos))
                    drawList->AddCircleFilled(pos, 2.0f, IM_COL32(255, 255, 0, 255));
            }
        }
    }

    // Draw Player (cyan)
    if (m_player)
    {
        ImVec2 pos = worldToMinimap(m_player->getPosition());
        if (isInsideCircle(pos))
            drawList->AddCircleFilled(pos, 4.0f, IM_COL32(0, 255, 255, 255));
    }
}

void GameUI::drawHealthBars()
{
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Position below minimap
    float startX = 30.0f;
    float startY = 30.0f;  
    float barWidth = 250.0f;
    float barHeight = 20.0f;
    float spacing = 28.0f;

    // Helper to draw a health bar
    auto drawHealthBar = [&](float y, Core* core, ImU32 color, const char* label) {
        if (!core) return;

        float healthPercent = core->isAlive() ? (core->getHealth() / core->getMaxHealth()) : 0.0f;

        // Background
        drawList->AddRectFilled(
            ImVec2(startX, y),
            ImVec2(startX + barWidth, y + barHeight),
            IM_COL32(40, 40, 40, 200)
        );

        // Health fill
        if (healthPercent > 0.0f)
        {
            drawList->AddRectFilled(
                ImVec2(startX, y),
                ImVec2(startX + barWidth * healthPercent, y + barHeight),
                color
            );
        }

        // Border
        drawList->AddRect(
            ImVec2(startX, y),
            ImVec2(startX + barWidth, y + barHeight),
            IM_COL32(100, 100, 100, 255)
        );
        };

    // Draw three core health bars
    drawHealthBar(startY, m_coreRed, IM_COL32(255, 60, 60, 255), "RED");
    drawHealthBar(startY + spacing, m_coreGreen, IM_COL32(60, 255, 60, 255), "GREEN");
    drawHealthBar(startY + spacing * 2, m_coreBlue, IM_COL32(60, 120, 255, 255), "BLUE");
}

void GameUI::drawBoostIndicator()
{
    if (!m_player) return;

    // Only show during PlayerDestruction phases
    //if (!m_musicManager || !m_musicManager->isPlayerDestructionPhase()) return;
    if (!m_musicManager) return;

    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);
    float screenHeight = static_cast<float>(size.bottom - size.top);

    // Bar dimensions
    float barWidth = 36.0f;
    float barHeight = 320.0f;

    // Position: slightly right of center, above beat bar
    float barX = (screenWidth / 2.0f) + 850.0f;  // 320px right of center
    float barY = screenHeight - 720.0f;          // Above the beat bar

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // Get boost state
    bool isBoosting = m_player->isBoosting();
    float boostTimer = m_player->getBoostTimer();
    float boostDuration = m_player->getBoostDuration();
    float boostPercent = isBoosting ? (boostTimer / boostDuration) : 1.0f;

    // Background
    drawList->AddRectFilled(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        IM_COL32(30, 30, 30, 200)
    );

    // Boost fill (from bottom up)
    float fillHeight = barHeight * boostPercent;
    float fillY = barY + barHeight - fillHeight;  // Start from bottom

    // Color: Cyan when ready, fading when depleting
    ImU32 fillColor;
    if (isBoosting)
    {
        // Pulsing cyan while active
        int alpha = 150 + (int)(105 * boostPercent);
        fillColor = IM_COL32(0, 255, 255, alpha);
    }
    else
    {
        // Solid cyan when ready
        fillColor = IM_COL32(0, 255, 255, 255);
    }

    drawList->AddRectFilled(
        ImVec2(barX, fillY),
        ImVec2(barX + barWidth, barY + barHeight),
        fillColor
    );

    // Border
    drawList->AddRect(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        IM_COL32(100, 100, 100, 255),
        0.0f,
        0,
        2.0f
    );

    // "BOOST" label (optional)
    // drawList->AddText(ImVec2(barX - 10, barY + barHeight + 5), IM_COL32(255, 255, 255, 200), "BOOST");
}

void GameUI::drawSongProgress()
{
    if (!m_musicManager) return;

    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);

    float timeRemaining = m_musicManager->getSongDuration() - m_musicManager->getElapsedTime();

    // Format time as M:SS
    int minutes = (int)timeRemaining / 60;
    int seconds = (int)timeRemaining % 60;

    // Draw at top-right
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    char timeText[32];
    sprintf_s(timeText, "%d:%02d", minutes, seconds);

    // Position at top-right
    float x = screenWidth / 2.0f - 100.0f;  // Center
    float y = 10.0f;

    ImFont* font = ImGui::GetFont();

    drawList->AddText(font, 48.0f, ImVec2(x, y + 30), IM_COL32(0, 255, 255, 255), timeText);  // Timer even bigger
}

void GameUI::drawCrosshair()
{
    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);
    float screenHeight = static_cast<float>(size.bottom - size.top);

    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;

    float lineLength = 16.0f;
    float lineGap = 6.0f;
    float lineThickness = 3.0f;

    ImU32 crosshairColor = IM_COL32(0, 255, 255, 200);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // === COOLDOWN RING ===
    if (m_beamWeapon)
    {
        float cooldownProgress = m_beamWeapon->getCooldownProgress();
        float ringRadius = 25.0f;
        float ringThickness = 4.0f;

        if (cooldownProgress > 0.0f)
        {
            // Background ring (dark)
            drawList->AddCircle(
                ImVec2(centerX, centerY),
                ringRadius,
                IM_COL32(50, 50, 50, 150),
                32,
                ringThickness
            );

            // Cooldown arc (fills as cooldown completes)
            float readyProgress = 1.0f - cooldownProgress;  // 0 = just fired, 1 = ready
            float startAngle = -XM_PI / 2.0f;  // Start from top
            float endAngle = startAngle + (readyProgress * XM_PI * 2.0f);

            // Draw arc using path
            int segments = (int)(readyProgress * 32);
            if (segments > 1)
            {
                for (int i = 0; i < segments; i++)
                {
                    float a1 = startAngle + (float(i) / 32.0f) * XM_PI * 2.0f;
                    float a2 = startAngle + (float(i + 1) / 32.0f) * XM_PI * 2.0f;
                    if (a2 > endAngle) a2 = endAngle;

                    drawList->AddLine(
                        ImVec2(centerX + cosf(a1) * ringRadius, centerY + sinf(a1) * ringRadius),
                        ImVec2(centerX + cosf(a2) * ringRadius, centerY + sinf(a2) * ringRadius),
                        IM_COL32(0, 255, 255, 255),
                        ringThickness
                    );
                }
            }

            // Dim crosshair while on cooldown
            crosshairColor = IM_COL32(0, 255, 255, 100);
        }
    }

    // Draw horizontal line (left segment)
    drawList->AddLine(
        ImVec2(centerX - lineGap - lineLength, centerY),
        ImVec2(centerX - lineGap, centerY),
        crosshairColor,
        lineThickness
    );

    // Draw horizontal line (right segment)
    drawList->AddLine(
        ImVec2(centerX + lineGap, centerY),
        ImVec2(centerX + lineGap + lineLength, centerY),
        crosshairColor,
        lineThickness
    );

    // Draw vertical line (top segment)
    drawList->AddLine(
        ImVec2(centerX, centerY - lineGap - lineLength),
        ImVec2(centerX, centerY - lineGap),
        crosshairColor,
        lineThickness
    );

    // Draw vertical line (bottom segment)
    drawList->AddLine(
        ImVec2(centerX, centerY + lineGap),
        ImVec2(centerX, centerY + lineGap + lineLength),
        crosshairColor,
        lineThickness
    );
}

void GameUI::triggerBeatFlash(int beat)
{
    m_edgeGlowAlpha = 1.0f;  // Full brightness

    // testing
    m_beatPulseAlpha = 1.0f;

    // odd beats (1,3) = cyan
    // even beats (2, 4) = magenta

    if (beat % 2 == 1)
        m_beatColor = IM_COL32(0, 255, 255, 255);   // cyan
    else
        m_beatColor = IM_COL32(255, 0, 255, 255);   // magenta 
}