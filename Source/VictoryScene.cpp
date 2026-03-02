#include "pch.h"
#include "VictoryScene.h"
#include "SceneManager.h"
#include "InputManager.h"

VictoryScene::VictoryScene(SceneManager* sceneManager)
	: Scene("Victory")
	, m_sceneManager(sceneManager)
    , m_selectedIndex(0)
{
}

VictoryScene::~VictoryScene()
{
}

void VictoryScene::initialize(DX::DeviceResources* deviceResources)
{
    Scene::initialize(deviceResources);

    // Create audio manager instance
    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadMusic("victory", GetAssetPath(L"Audio/victory_sfx.wav").c_str());
    
    // Shader Background
    m_background = std::make_unique<MenuBackground>(deviceResources);
    m_background->initialize();
    m_background->setShaderType(MenuBackground::ShaderType::Wave);
    m_background->setColorTint(1.0f, 0.85f, 0.4f);  // Gold
}

void VictoryScene::enter()
{
	Scene::enter();
    m_selectedIndex = 0;
    // playmusic with loop = true 
    if (m_audioManager) m_audioManager->playMusic("victory", true);
}

void VictoryScene::exit()
{
	Scene::exit();
    if (m_audioManager) m_audioManager->stopMusic();
}

void VictoryScene::finalize()
{
    if (m_background) m_background->onDeviceLost();
}

void VictoryScene::update(float deltaTime, InputManager* input)
{
    if (m_audioManager) m_audioManager->update();

    // Update background animation
    if (m_background) m_background->update(deltaTime);

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

    // TransitionTo (with fading transition effect)
    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space) || input->isGamePadBPressed())
    {
        switch (m_selectedIndex)
        {
        case 0:  // PLAY AGAIN
            m_sceneManager->transitionTo("GameScene");
            break;
        case 1:  // MAIN MENU
            m_sceneManager->transitionTo("MainMenu");
            break;
        }
    }
}

void VictoryScene::render(Renderer* renderer)
{
    (void)renderer;

    // Render shader background
    if (m_background) m_background->render();

    // ImGui UI
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float centerX = windowSize.x * 0.5f;
    float centerY = windowSize.y * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 350));

    ImGui::Begin("##Victory", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);

    float windowWidth = ImGui::GetWindowWidth();

    // Title
    ImGui::SetWindowFontScale(4.0f);
    const char* title = "VICTORY";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(title).x) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), title);

    ImGui::SetWindowFontScale(1.5f);
    const char* subtitle = "SYSTEM SECURED";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(subtitle).x) / 2.0f);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), subtitle);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Menu
    ImGui::SetWindowFontScale(1.8f);

    const char* menuItems[] = { "PLAY AGAIN", "MAIN MENU" };

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
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), displayText);  // Gold
        else
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), displayText);

        ImGui::Spacing();
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

