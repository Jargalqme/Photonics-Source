#include "pch.h"
#include "GameOverScene.h"
#include "SceneManager.h"
#include "InputManager.h"

GameOverScene::GameOverScene(SceneManager* sceneManager)
	: Scene("GameOver")
	, m_sceneManager(sceneManager)
    , m_selectedIndex(0)
{
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::Initialize(DX::DeviceResources* deviceResources)
{
    m_deviceResources = deviceResources;

    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadMusic("gameover", GetAssetPath(L"Audio/gameover_sfx.wav").c_str());
    
    // === Shader Background ===
    m_background = std::make_unique<MenuBackground>(deviceResources);
    m_background->Initialize();
    m_background->SetShaderType(MenuBackground::ShaderType::Wave);
    m_background->SetColorTint(1.0f, 0.2f, 0.2f);  // Red
}

void GameOverScene::Enter()
{
	Scene::Enter();
    m_selectedIndex = 0;
    if (m_audioManager) m_audioManager->playMusic("gameover", true);
}

void GameOverScene::Exit()
{
	Scene::Exit();
    if (m_audioManager) m_audioManager->stopMusic();
}

void GameOverScene::Cleanup()
{

}

void GameOverScene::Update(float deltaTime, InputManager* input)
{
    if (m_audioManager) m_audioManager->update();

    // Update background animation  <-- ADD
    if (m_background) m_background->Update(deltaTime);

    // Navigation
    if (input->isKeyPressed(Keyboard::Keys::Up) || input->isKeyPressed(Keyboard::Keys::W) || input->isGamePadDPadUpPressed())
    {
        m_selectedIndex--;
        if (m_selectedIndex < 0) m_selectedIndex = m_menuItemCount - 1;
    }
    if (input->isKeyPressed(Keyboard::Keys::Down) || input->isKeyPressed(Keyboard::Keys::S) || input->isGamePadDPadDownPressed())
    {
        m_selectedIndex++;
        if (m_selectedIndex >= m_menuItemCount) m_selectedIndex = 0;
    }

    // Selection - USE TransitionTo
    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space) || input->isGamePadBPressed())
    {
        switch (m_selectedIndex)
        {
        case 0:  // RETRY
            m_sceneManager->TransitionTo("GameScene");
            break;
        case 1:  // MAIN MENU
            m_sceneManager->TransitionTo("MainMenu");
            break;
        }
    }
}

void GameOverScene::Render(Renderer* renderer)
{
    (void)renderer;

    if (m_background) m_background->Render();

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

    // === TITLE ===
    ImGui::SetWindowFontScale(4.0f);
    const char* title = "GAME OVER";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(title).x) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), title);

    ImGui::SetWindowFontScale(1.5f);
    const char* subtitle = "SYSTEM COMPROMISED";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(subtitle).x) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), subtitle);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // === MENU ITEMS ===
    ImGui::SetWindowFontScale(1.8f);

    const char* menuItems[] = { "RETRY", "MAIN MENU" };

    for (int i = 0; i < m_menuItemCount; i++)
    {
        bool isSelected = (i == m_selectedIndex);

        char displayText[64];
        if (isSelected)
            sprintf_s(displayText, "> %s <", menuItems[i]);
        else
            sprintf_s(displayText, "  %s  ", menuItems[i]);

        float textWidth = ImGui::CalcTextSize(displayText).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) / 2.0f);

        if (isSelected)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), displayText);  // Red
        else
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), displayText);

        ImGui::Spacing();
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

void GameOverScene::OnDeviceLost()
{
    if (m_background) m_background->OnDeviceLost();
}

void GameOverScene::OnDeviceRestored()
{
    if (m_background)
    {
        m_background->SetShaderType(MenuBackground::ShaderType::Wave);
        m_background->SetColorTint(1.0f, 0.2f, 0.2f);
    }
}