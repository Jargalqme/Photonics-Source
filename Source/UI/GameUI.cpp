#include "pch.h"
#include "UI/GameUI.h"
#include "Services/BeatTracker.h"
#include "Gameplay/Player.h"
#include "Gameplay/Dummy.h"
#include "Gameplay/Boss.h"
#include "Gameplay/Events/EventBus.h"
#include "Gameplay/Events/EventTypes.h"
#include <imgui.h>

GameUI::GameUI()
{
}

void GameUI::initialize(DX::DeviceResources* deviceResources)
{
    m_deviceResources = deviceResources;
}

void GameUI::subscribeEvents()
{
    EventBus::subscribe<WaveChangedEvent>([this](const WaveChangedEvent& event) {
        setWaveNumber(event.waveNumber);
    });
}

// === 更新 ===

void GameUI::update(float deltaTime)
{
    // エッジグローの減衰
    if (m_edgeGlowAlpha > 0.0f)
    {
        m_edgeGlowAlpha -= EDGE_GLOW_DECAY * deltaTime;
        if (m_edgeGlowAlpha < 0.0f)
        {
            m_edgeGlowAlpha = 0.0f;
        }
    }

    // ビートパルスの減衰
    if (m_beatPulseAlpha > 0.0f)
    {
        m_beatPulseAlpha -= BEAT_PULSE_DECAY * deltaTime;
        if (m_beatPulseAlpha < 0.0f)
        {
            m_beatPulseAlpha = 0.0f;
        }
    }
}

// === 描画 ===

void GameUI::render(const Matrix& view, const Matrix& proj)
{
    drawDummyHealthBar(view, proj);
    drawMinimap();
    drawCrosshair();
    if (m_showWaveIndicator)
    {
        drawWaveIndicator();
    }
    drawPlayerHealth();
    drawBossHealth();
    drawWeaponHUD();
}

// === ウェーブ表示 ===

void GameUI::drawWaveIndicator()
{
    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);

    const char* text = "WAVE 1";
    if (m_currentWave == 2)
    {
        text = "WAVE 2";
    }
    else if (m_currentWave == 3)
    {
        text = "BOSS";
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetFont();

    ImVec2 textSize = font->CalcTextSizeA(36.0f, FLT_MAX, 0.0f, text);
    float x = (screenWidth - textSize.x) / 2.0f;
    float y = 10.0f;

    drawList->AddText(font, 36.0f, ImVec2(x, y), IM_COL32(0, 255, 255, 255), text);
}

// === ミニマップ ===

void GameUI::drawMinimap()
{
    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);

    // ミニマップ配置（円形）
    float mapRadius = 120.0f;
    float mapCenterX = screenWidth - mapRadius - 30.0f;
    float mapCenterY = mapRadius + 30.0f;
    float worldSize = 100.0f;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // 円形背景
    drawList->AddCircleFilled(
        ImVec2(mapCenterX, mapCenterY),
        mapRadius,
        IM_COL32(20, 20, 20, 200),
        32
    );

    // 枠線
    drawList->AddCircle(
        ImVec2(mapCenterX, mapCenterY),
        mapRadius,
        IM_COL32(100, 100, 100, 255),
        32,
        2.0f
    );

    // プレイヤー相対回転
    Vector3 playerPos = m_player->getPosition();
    Vector3 fwd = m_player->getForward();
    float sinYaw = fwd.x;
    float cosYaw = fwd.z;

    auto worldToMinimap = [&](Vector3 worldPos) -> ImVec2
    {
        // プレイヤーからのオフセット（プレイヤーが中心）
        float dx = worldPos.x - playerPos.x;
        float dz = worldPos.z - playerPos.z;

        // ヨーの逆回転（プレイヤーの前方が上向き）
        float rx = dx * cosYaw - dz * sinYaw;
        float ry = dx * sinYaw + dz * cosYaw;

        float x = (rx / worldSize) * mapRadius + mapCenterX;
        float y = (-ry / worldSize) * mapRadius + mapCenterY;
        return ImVec2(x, y);
    };

    auto isInsideCircle = [&](ImVec2 pos) -> bool
    {
        float dx = pos.x - mapCenterX;
        float dy = pos.y - mapCenterY;
        return (dx * dx + dy * dy) <= (mapRadius * mapRadius);
    };

    // 敵（黄色の点）
    if (m_dummies)
    {
        for (auto& dummy : *m_dummies)
        {
            if (dummy->isActive())
            {
                ImVec2 pos = worldToMinimap(dummy->getPosition());
                if (isInsideCircle(pos))
                {
                    drawList->AddCircleFilled(pos, 2.0f, IM_COL32(255, 255, 0, 255));
                }
            }
        }
    }

    // ボス（マゼンタ）
    if (m_boss && m_boss->isActivated() && !m_boss->isDead())
    {
        ImVec2 pos = worldToMinimap(m_boss->getPosition());
        if (isInsideCircle(pos))
        {
            drawList->AddCircleFilled(pos, 5.0f, IM_COL32(255, 0, 255, 255));
        }
    }

    // プレイヤー（シアン三角形）
    if (m_player)
    {
        float triSize = 8.0f;
        ImVec2 center(mapCenterX, mapCenterY);

        drawList->AddTriangleFilled(
            ImVec2(center.x, center.y - triSize),
            ImVec2(center.x - triSize * 0.6f, center.y + triSize * 0.5f),
            ImVec2(center.x + triSize * 0.6f, center.y + triSize * 0.5f),
            IM_COL32(0, 255, 255, 255)
        );
    }
}

// === 照準 ===

void GameUI::drawCrosshair()
{
    if (m_player && m_player->isAiming())
    {
        return;
    }

    auto size = m_deviceResources->GetOutputSize();
    float centerX = static_cast<float>(size.right - size.left) / 2.0f;
    float centerY = static_cast<float>(size.bottom - size.top) / 2.0f;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    drawList->AddCircle(ImVec2(centerX, centerY), 20.0f, IM_COL32(255, 0, 255, 255), 0, 2.0f);
}

// === ビートフラッシュ ===

void GameUI::triggerBeatFlash(int beat)
{
    m_edgeGlowAlpha = 1.0f;
    m_beatPulseAlpha = 1.0f;

    // 奇数ビート = シアン、偶数ビート = マゼンタ
    if (beat % 2 == 1)
    {
        m_beatColor = IM_COL32(0, 255, 255, 255);
    }
    else
    {
        m_beatColor = IM_COL32(255, 0, 255, 255);
    }
}

// === プレイヤー体力バー ===

void GameUI::drawPlayerHealth()
{
    if (!m_player)
    {
        return;
    }

    auto size = m_deviceResources->GetOutputSize();
    float screenHeight = static_cast<float>(size.bottom - size.top);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    float barWidth = 350.0f;
    float barHeight = 18.0f;
    float x = 30.0f;
    float y = screenHeight - barHeight - 30.0f;

    float healthPercent = m_player->getHealth() / m_player->getMaxHealth();

    // 体力に応じた色
    ImU32 fillColor;
    if (healthPercent > 0.5f)
    {
        fillColor = IM_COL32(0, 220, 0, 255);
    }
    else if (healthPercent > 0.25f)
    {
        fillColor = IM_COL32(220, 220, 0, 255);
    }
    else
    {
        fillColor = IM_COL32(220, 0, 0, 255);
    }

    // 背景
    drawList->AddRectFilled(
        ImVec2(x, y),
        ImVec2(x + barWidth, y + barHeight),
        IM_COL32(40, 40, 40, 200)
    );

    // 体力ゲージ
    if (healthPercent > 0.0f)
    {
        drawList->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + barWidth * healthPercent, y + barHeight),
            fillColor
        );
    }

    // 枠線
    drawList->AddRect(
        ImVec2(x, y),
        ImVec2(x + barWidth, y + barHeight),
        IM_COL32(100, 100, 100, 255)
    );

    // ラベル
    ImFont* font = ImGui::GetFont();
    drawList->AddText(font, 14.0f, ImVec2(x, y - 16.0f), IM_COL32(255, 255, 255, 200), "PLAYER");
}

// === ボス体力バー ===

void GameUI::drawBossHealth()
{
    if (!m_boss || !m_boss->isActivated() || m_boss->isDead())
    {
        return;
    }

    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    float barWidth = 600.0f;
    float barHeight = 16.0f;
    float x = (screenWidth - barWidth) / 2.0f;
    float y = 60.0f;

    float healthPercent = m_boss->getHealthPercent();

    // 背景
    drawList->AddRectFilled(
        ImVec2(x, y),
        ImVec2(x + barWidth, y + barHeight),
        IM_COL32(40, 40, 40, 200)
    );

    // 体力ゲージ（赤）
    if (healthPercent > 0.0f)
    {
        drawList->AddRectFilled(
            ImVec2(x, y),
            ImVec2(x + barWidth * healthPercent, y + barHeight),
            IM_COL32(220, 0, 40, 255)
        );
    }

    // 枠線
    drawList->AddRect(
        ImVec2(x, y),
        ImVec2(x + barWidth, y + barHeight),
        IM_COL32(100, 100, 100, 255)
    );

    // HPテキスト
    ImFont* font = ImGui::GetFont();
    char hpText[32];
    sprintf_s(hpText, "BOSS  %.0f / %.0f", m_boss->getHealth(), m_boss->getMaxHealth());
    ImVec2 textSize = font->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, hpText);
    float textX = x + (barWidth - textSize.x) / 2.0f;
    drawList->AddText(font, 16.0f, ImVec2(textX, y - 20.0f), IM_COL32(255, 255, 255, 255), hpText);
}

// === 武器HUD ===

void GameUI::drawWeaponHUD()
{
    if (!m_player)
    {
        return;
    }

    auto size = m_deviceResources->GetOutputSize();
    float screenWidth = static_cast<float>(size.right - size.left);
    float screenHeight = static_cast<float>(size.bottom - size.top);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetFont();

    float x = screenWidth - 280.0f;
    float y = screenHeight - 60.0f;

    // 武器名
    drawList->AddText(font, 20.0f, ImVec2(x, y),
        IM_COL32(255, 255, 255, 150), "BOLT RIFLE");

    // 弾数
    char ammoText[32];
    if (m_player->isReloading())
    {
        sprintf_s(ammoText, "RELOADING");
    }
    else
    {
        sprintf_s(ammoText, "%d / %d", m_player->getAmmo(), m_player->getMaxAmmo());
    }

    ImU32 ammoColor = (m_player->getAmmo() <= 5)
        ? IM_COL32(255, 50, 50, 255)
        : IM_COL32(255, 255, 255, 255);

    drawList->AddText(font, 42.0f, ImVec2(x, y + 14.0f), ammoColor, ammoText);
}

// === 敵体力バー（ワールド空間） ===

void GameUI::drawDummyHealthBar(const Matrix& view, const Matrix& proj)
{
    if (!m_dummies)
    {
        return;
    }

    auto size = m_deviceResources->GetOutputSize();
    float screenW = static_cast<float>(size.right);
    float screenH = static_cast<float>(size.bottom);

    auto* drawList = ImGui::GetForegroundDrawList();

    for (auto& dummy : *m_dummies)
    {
        if (!dummy->isActive())
        {
            continue;
        }

        // 敵の頭上にバーを配置
        Vector3 worldPos = dummy->getPosition() + Vector3(0.0f, 2.5f, 0.0f);

        // ワールド → クリップ空間
        Vector4 clip = Vector4::Transform(
            Vector4(worldPos.x, worldPos.y, worldPos.z, 1.0f),
            view * proj
        );

        // カメラの後ろならスキップ
        if (clip.w <= 0.0f)
        {
            continue;
        }

        // 透視除算 → NDC
        float ndcX = clip.x / clip.w;
        float ndcY = clip.y / clip.w;

        // NDC → スクリーン座標
        float screenX = (ndcX * 0.5f + 0.5f) * screenW;
        float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * screenH;

        // 距離に応じたスケール
        float scale = 1.0f / clip.w;
        scale = std::clamp(scale, 0.05f, 0.5f);

        float barW = 600.0f * scale;
        float barH = 60.0f * scale;
        float left = screenX - barW * 0.5f;
        float top = screenY - barH * 0.5f;

        float ratio = dummy->getHealth() / dummy->getMaxHealth();

        // 背景
        drawList->AddRectFilled(
            ImVec2(left, top),
            ImVec2(left + barW, top + barH),
            IM_COL32(0, 0, 0, 160)
        );

        // 体力ゲージ（緑→赤）
        ImU32 color = (ratio > 0.5f)
            ? IM_COL32(0, 255, 100, 220)
            : IM_COL32(255, 60, 30, 220);

        drawList->AddRectFilled(
            ImVec2(left, top),
            ImVec2(left + barW * ratio, top + barH),
            color
        );
    }
}
