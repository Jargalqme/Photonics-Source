#include "pch.h"
#include "Scenes/GameOverScene.h"
#include "Scenes/SceneManager.h"
#include "Services/InputManager.h"
#include "Renderer.h"

// === 生成・破棄 ===

GameOverScene::GameOverScene(SceneManager* sceneManager)
    : Scene("GameOver")
    , m_sceneManager(sceneManager)
    , m_selectedIndex(0)
{
}

GameOverScene::~GameOverScene()
{
}

// === シーンライフサイクル ===

void GameOverScene::initialize(SceneContext& context)
{
    Scene::initialize(context);

    // SE
    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadMusic("gameover", GetAssetPath(L"Audio/gameover_sfx.wav").c_str());

    // 波形シェーダー背景（赤系）
    m_background = std::make_unique<MenuBackground>(m_deviceResources);
    m_background->initialize();
    m_background->setShaderType(MenuBackground::ShaderType::Wave);
    m_background->setColorTint(1.0f, 0.2f, 0.2f);
}

void GameOverScene::enter()
{
    Scene::enter();
    m_selectedIndex = 0;

    if (m_audioManager)
    {
        m_audioManager->playMusic("gameover", true);
    }
}

void GameOverScene::exit()
{
    Scene::exit();

    if (m_audioManager)
    {
        m_audioManager->stopMusic();
    }
}

void GameOverScene::finalize()
{
    if (m_background)
    {
        m_background->onDeviceLost();
    }
}

// === 更新 ===

void GameOverScene::update(float deltaTime, InputManager* input)
{
    if (m_audioManager)
    {
        m_audioManager->update();
    }
    if (m_background)
    {
        m_background->update(deltaTime);
    }

    // --- ナビゲーション ---
    if (input->isKeyPressed(Keyboard::Keys::Up) || input->isKeyPressed(Keyboard::Keys::W))
    {
        m_selectedIndex--;
        if (m_selectedIndex < 0)
        {
            m_selectedIndex = m_menuItemCount - 1;
        }
    }
    if (input->isKeyPressed(Keyboard::Keys::Down) || input->isKeyPressed(Keyboard::Keys::S))
    {
        m_selectedIndex++;
        if (m_selectedIndex >= m_menuItemCount)
        {
            m_selectedIndex = 0;
        }
    }

    // --- 決定 ---
    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space))
    {
        switch (m_selectedIndex)
        {
        case 0:  // リトライ
            m_sceneManager->transitionTo("BossScene");
            break;
        case 1:  // メインメニュー
            m_sceneManager->transitionTo("MainMenu");
            break;
        }
    }
}

// === 描画 ===

void GameOverScene::render()
{
    if (m_background)
    {
        m_background->render(m_renderer->GetRenderWidth(), m_renderer->GetRenderHeight());
    }

    // === ImGui メニュー ===
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float centerX = windowSize.x * 0.5f;
    float centerY = windowSize.y * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 350));

    ImGui::Begin("##GameOver", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);

    float windowWidth = ImGui::GetWindowWidth();

    // タイトル
    ImGui::SetWindowFontScale(4.0f);
    const char* title = "GAME OVER";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(title).x) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), title);

    // サブタイトル
    ImGui::SetWindowFontScale(1.5f);
    const char* subtitle = "SYSTEM COMPROMISED";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(subtitle).x) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), subtitle);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // メニュー項目
    ImGui::SetWindowFontScale(1.8f);

    const char* menuItems[] = { "RETRY", "MAIN MENU" };

    for (int i = 0; i < m_menuItemCount; i++)
    {
        bool isSelected = (i == m_selectedIndex);

        char displayText[64];
        if (isSelected)
        {
            sprintf_s(displayText, "> %s <", menuItems[i]);
        }
        else
        {
            sprintf_s(displayText, "  %s  ", menuItems[i]);
        }

        float textWidth = ImGui::CalcTextSize(displayText).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) / 2.0f);

        if (isSelected)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), displayText);
        }
        else
        {
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), displayText);
        }

        ImGui::Spacing();
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}
