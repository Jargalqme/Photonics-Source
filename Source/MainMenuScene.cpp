#include "pch.h"
#include "MainMenuScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "Renderer.h"

MainMenuScene::MainMenuScene(SceneManager* sceneManager)
	: Scene("MainMenu")
	, m_sceneManager(sceneManager)
    , m_selectedIndex(0)
{
}

MainMenuScene::~MainMenuScene()
{
}

void MainMenuScene::initialize(DX::DeviceResources* deviceResources)
{
	Scene::initialize(deviceResources);

    // Create background
    m_background = std::make_unique<MenuBackground>(deviceResources);
    m_background->initialize();
    m_background->setShaderType(MenuBackground::ShaderType::Wave);
    m_background->setColorTint(0.8f, 0.5f, 0.8f);

    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadMusic("menu", GetAssetPath(L"Audio/menu_music.wav").c_str());
}

void MainMenuScene::enter()
{
    Scene::enter();
    m_selectedIndex = 0;
    if (m_audioManager) m_audioManager->playMusic("menu", true);
}

void MainMenuScene::exit()
{
	Scene::exit();
    if (m_audioManager) m_audioManager->stopMusic();
}

void MainMenuScene::finalize()
{
    if (m_background) m_background->onDeviceLost();
}

void MainMenuScene::update(float deltaTime, InputManager* input)
{
    if (m_audioManager) m_audioManager->update();
    if (m_background) m_background->update(deltaTime);

    // NAVIGATION (only when menu visible)
    if (input->isKeyPressed(Keyboard::Keys::Up) || input->isKeyPressed(Keyboard::Keys::W) || input->isGamePadDPadUpPressed())
    {
        m_selectedIndex--;
        if (m_selectedIndex < 0)
            m_selectedIndex = m_menuItemCount - 1;
    }

    if (input->isKeyPressed(Keyboard::Keys::Down) || input->isKeyPressed(Keyboard::Keys::S) || input->isGamePadDPadDownPressed())
    {
        m_selectedIndex++;
        if (m_selectedIndex >= m_menuItemCount)
            m_selectedIndex = 0;
    }

    // SELECTION
    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space) || input->isGamePadBPressed())
    {
        switch (m_selectedIndex)
        {
        case 0:  // START GAME
            m_sceneManager->transitionTo("GameScene");
            break;
        case 1:  // QUIT
            PostQuitMessage(0);
            break;
        }
    }

    if (input->isKeyPressed(Keyboard::Keys::Escape))
    {
        PostQuitMessage(0);
    }
}

void MainMenuScene::render(Renderer* renderer)
{
    (void)renderer;
    if (m_background) m_background->render();
    renderMenu();
}

void DrawSciFiButton(const char* text, bool isSelected, float width, float centerX)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float height = 44.0f;
    float x = centerX - width / 2.0f;
    float y = ImGui::GetCursorScreenPos().y;

    // Sci-Fi Cyan Colors
    ImU32 bgColor = isSelected ? IM_COL32(0, 60, 80, 220) : IM_COL32(0, 20, 30, 200);
    ImU32 borderColor = isSelected ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 150, 150, 180);
    ImU32 glowColor = IM_COL32(0, 255, 255, isSelected ? 60 : 0);
    ImU32 textColor = isSelected ? IM_COL32(0, 255, 255, 255) : IM_COL32(150, 200, 200, 255);

    // Outer glow (selected only)
    if (isSelected)
    {
        drawList->AddRectFilled(
            ImVec2(x - 4, y - 4),
            ImVec2(x + width + 4, y + height + 4),
            glowColor,
            6.0f
        );
    }

    // Background
    drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), bgColor, 4.0f);

    // Border
    drawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), borderColor, 4.0f, 0, isSelected ? 2.0f : 1.0f);

    // Corner accents (sci-fi detail)
    float cornerSize = 8.0f;
    // Top-left
    drawList->AddLine(ImVec2(x, y + cornerSize), ImVec2(x, y), borderColor, 2.0f);
    drawList->AddLine(ImVec2(x, y), ImVec2(x + cornerSize, y), borderColor, 2.0f);
    // Top-right
    drawList->AddLine(ImVec2(x + width - cornerSize, y), ImVec2(x + width, y), borderColor, 2.0f);
    drawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + cornerSize), borderColor, 2.0f);
    // Bottom-left
    drawList->AddLine(ImVec2(x, y + height - cornerSize), ImVec2(x, y + height), borderColor, 2.0f);
    drawList->AddLine(ImVec2(x, y + height), ImVec2(x + cornerSize, y + height), borderColor, 2.0f);
    // Bottom-right
    drawList->AddLine(ImVec2(x + width - cornerSize, y + height), ImVec2(x + width, y + height), borderColor, 2.0f);
    drawList->AddLine(ImVec2(x + width, y + height - cornerSize), ImVec2(x + width, y + height), borderColor, 2.0f);

    // Centered text
    ImVec2 textSize = ImGui::CalcTextSize(text);
    float textX = x + (width - textSize.x) / 2.0f;
    float textY = y + (height - textSize.y) / 2.0f;
    drawList->AddText(ImVec2(textX, textY), textColor, text);

    // Advance cursor
    ImGui::Dummy(ImVec2(width, height + 12.0f));
}

void MainMenuScene::renderMenu()
{
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float centerX = windowSize.x * 0.5f;
    float centerY = windowSize.y * 0.5f;

    float panelWidth = 420.0f;
    float panelHeight = 320.0f;

    // === MENU WINDOW ===
    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGui::Begin("##MainMenu", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);

    float windowWidth = ImGui::GetWindowWidth();

    // === TITLE ===
    ImGui::SetWindowFontScale(4.0f);
    const char* title = "P H O T O N";
    float titleWidth = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((windowWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), title);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // === MENU ITEMS ===
    ImGui::SetWindowFontScale(1.8f);

    const char* menuItems[] = { "START GAME", "QUIT" };

    for (int i = 0; i < m_menuItemCount; i++)
    {
        DrawSciFiButton(menuItems[i], i == m_selectedIndex, 280.0f, centerX);
    }

    ImGui::SetWindowFontScale(1.0f);

    // === CONTROLS HINT ===
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.5f);
    const char* hint = "[W/S | D-PAD] Move  [ENTER | B] Select";
    float hintWidth = ImGui::CalcTextSize(hint).x;
    ImGui::SetCursorPosX((windowWidth - hintWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), hint);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
}

