#include "pch.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/SceneManager.h"
#include "Services/InputManager.h"
#include "Renderer.h"
#include "Game.h"

#include <cstdio>
#include <iterator>

namespace
{
    struct ResolutionOption
    {
        int width;
        int height;
        const char* label;
    };

    constexpr ResolutionOption RESOLUTIONS[] = {
        { 1920, 1080, "1920 x 1080" },
        { 1920, 1200, "1920 x 1200" },
    };

    constexpr int RESOLUTION_COUNT = static_cast<int>(std::size(RESOLUTIONS));

    void DrawSciFiButton(const char* text, bool isSelected, float width, float centerX)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const float height = 44.0f;
        const float x = centerX - width / 2.0f;
        const float y = ImGui::GetCursorScreenPos().y;

        const ImU32 bgColor = isSelected ? IM_COL32(0, 60, 80, 220) : IM_COL32(0, 20, 30, 200);
        const ImU32 borderColor = isSelected ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 150, 150, 180);
        const ImU32 glowColor = IM_COL32(0, 255, 255, isSelected ? 60 : 0);
        const ImU32 textColor = isSelected ? IM_COL32(0, 255, 255, 255) : IM_COL32(150, 200, 200, 255);

        if (isSelected)
        {
            drawList->AddRectFilled(
                ImVec2(x - 4, y - 4),
                ImVec2(x + width + 4, y + height + 4),
                glowColor,
                6.0f);
        }

        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), bgColor, 4.0f);
        drawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), borderColor, 4.0f, 0, isSelected ? 2.0f : 1.0f);

        const float cornerSize = 8.0f;
        drawList->AddLine(ImVec2(x, y + cornerSize), ImVec2(x, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y), ImVec2(x + cornerSize, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width - cornerSize, y), ImVec2(x + width, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + cornerSize), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y + height - cornerSize), ImVec2(x, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y + height), ImVec2(x + cornerSize, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width - cornerSize, y + height), ImVec2(x + width, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width, y + height - cornerSize), ImVec2(x + width, y + height), borderColor, 2.0f);

        ImVec2 textSize = ImGui::CalcTextSize(text);
        const float textX = x + (width - textSize.x) / 2.0f;
        const float textY = y + (height - textSize.y) / 2.0f;
        drawList->AddText(ImVec2(textX, textY), textColor, text);

        ImGui::Dummy(ImVec2(width, height + 12.0f));
    }
}

MainMenuScene::MainMenuScene(SceneManager* sceneManager)
    : Scene("MainMenu")
    , m_sceneManager(sceneManager)
    , m_selectedIndex(0)
{
}

MainMenuScene::~MainMenuScene()
{
}

void MainMenuScene::initialize(SceneContext& context)
{
    Scene::initialize(context);

    m_background = std::make_unique<MenuBackground>(m_deviceResources);
    m_background->initialize();
    m_background->setShaderType(MenuBackground::ShaderType::Wave);
    m_background->setColorTint(0.0f, 0.4f, 1.0f);

    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->initialize();
    m_audioManager->loadMusic("menu", GetAssetPath(L"Audio/menu_music.wav").c_str());
}

void MainMenuScene::enter()
{
    Scene::enter();
    m_state = State::Root;
    m_selectedIndex = 0;

    if (m_audioManager)
    {
        m_audioManager->playMusic("menu", true);
    }
}

void MainMenuScene::exit()
{
    Scene::exit();

    if (m_audioManager)
    {
        m_audioManager->stopMusic();
    }
}

void MainMenuScene::finalize()
{
    if (m_background)
    {
        m_background->onDeviceLost();
    }
}

void MainMenuScene::update(float deltaTime, InputManager* input)
{
    if (m_audioManager)
    {
        m_audioManager->update();
    }

    if (m_background)
    {
        m_background->update(deltaTime);
    }

    if (m_state == State::Root)
    {
        updateRoot(input);
    }
    else
    {
        updateSettings(input);
    }
}

void MainMenuScene::updateRoot(InputManager* input)
{
    if (input->isKeyPressed(Keyboard::Keys::Up) || input->isKeyPressed(Keyboard::Keys::W))
    {
        m_selectedIndex = (m_selectedIndex - 1 + ROOT_ITEM_COUNT) % ROOT_ITEM_COUNT;
    }

    if (input->isKeyPressed(Keyboard::Keys::Down) || input->isKeyPressed(Keyboard::Keys::S))
    {
        m_selectedIndex = (m_selectedIndex + 1) % ROOT_ITEM_COUNT;
    }

    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space))
    {
        switch (m_selectedIndex)
        {
        case 0:
            m_sceneManager->transitionTo("BossScene");
            break;
        case 1:
            m_sceneManager->transitionTo("Training");
            break;
        case 2:
            enterSettings();
            break;
        case 3:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
    }

    if (input->isKeyPressed(Keyboard::Keys::Escape))
    {
        PostQuitMessage(0);
    }
}

void MainMenuScene::updateSettings(InputManager* input)
{
    if (input->isKeyPressed(Keyboard::Keys::Up) || input->isKeyPressed(Keyboard::Keys::W))
    {
        m_selectedIndex = (m_selectedIndex - 1 + SETTINGS_ITEM_COUNT) % SETTINGS_ITEM_COUNT;
    }

    if (input->isKeyPressed(Keyboard::Keys::Down) || input->isKeyPressed(Keyboard::Keys::S))
    {
        m_selectedIndex = (m_selectedIndex + 1) % SETTINGS_ITEM_COUNT;
    }

    const bool leftPressed = input->isKeyPressed(Keyboard::Keys::Left) || input->isKeyPressed(Keyboard::Keys::A);
    const bool rightPressed = input->isKeyPressed(Keyboard::Keys::Right) || input->isKeyPressed(Keyboard::Keys::D);
    if ((leftPressed || rightPressed) && m_selectedIndex == 0)
    {
        const int delta = rightPressed ? 1 : -1;
        m_pendingResolutionIndex = (m_pendingResolutionIndex + delta + RESOLUTION_COUNT) % RESOLUTION_COUNT;
    }

    if (input->isKeyPressed(Keyboard::Keys::Enter) || input->isKeyPressed(Keyboard::Keys::Space))
    {
        if (m_selectedIndex == 0)
        {
            m_pendingResolutionIndex = (m_pendingResolutionIndex + 1) % RESOLUTION_COUNT;
        }
        else if (m_selectedIndex == 1)
        {
            applySettings();
        }
        else if (m_selectedIndex == 2)
        {
            m_state = State::Root;
            m_selectedIndex = 2;
        }
    }

    if (input->isKeyPressed(Keyboard::Keys::Escape))
    {
        m_state = State::Root;
        m_selectedIndex = 2;
    }
}

void MainMenuScene::enterSettings()
{
    m_state = State::Settings;
    m_selectedIndex = 0;

    const int currentW = m_context->renderer->GetRenderWidth();
    const int currentH = m_context->renderer->GetRenderHeight();

    m_pendingResolutionIndex = 0;
    for (int i = 0; i < RESOLUTION_COUNT; i++)
    {
        if (RESOLUTIONS[i].width == currentW && RESOLUTIONS[i].height == currentH)
        {
            m_pendingResolutionIndex = i;
            break;
        }
    }
}

void MainMenuScene::applySettings()
{
    m_context->game->applyResolution(
        RESOLUTIONS[m_pendingResolutionIndex].width,
        RESOLUTIONS[m_pendingResolutionIndex].height);
}

void MainMenuScene::render()
{
    if (m_background)
    {
        m_background->render(m_renderer->GetRenderWidth(), m_renderer->GetRenderHeight());
    }

    if (m_state == State::Root)
    {
        renderRoot();
    }
    else
    {
        renderSettings();
    }
}

void MainMenuScene::renderRoot()
{
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    const float centerX = windowSize.x * 0.5f;
    const float centerY = windowSize.y * 0.5f;

    const float panelWidth = 420.0f;
    const float panelHeight = 360.0f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGui::Begin("##MainMenu", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);

    const float windowWidth = ImGui::GetWindowWidth();

    ImGui::SetWindowFontScale(4.0f);
    const char* title = "P H O T O N";
    const float titleWidth = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((windowWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), title);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.8f);

    const char* menuItems[ROOT_ITEM_COUNT] = { "BOSS FIGHT", "TRAINING", "SETTINGS", "QUIT" };
    for (int i = 0; i < ROOT_ITEM_COUNT; i++)
    {
        DrawSciFiButton(menuItems[i], i == m_selectedIndex, 280.0f, centerX);
    }

    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.5f);
    const char* hint = "[W/S] Move  [ENTER] Select";
    const float hintWidth = ImGui::CalcTextSize(hint).x;
    ImGui::SetCursorPosX((windowWidth - hintWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), hint);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
}

void MainMenuScene::renderSettings()
{
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    const float centerX = windowSize.x * 0.5f;
    const float centerY = windowSize.y * 0.5f;

    const float panelWidth = 520.0f;
    const float panelHeight = 340.0f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGui::Begin("##MainMenuSettings", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground);

    const float windowWidth = ImGui::GetWindowWidth();

    ImGui::SetWindowFontScale(3.0f);
    const char* title = "SETTINGS";
    const float titleWidth = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((windowWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), title);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.6f);

    char rowLabel[96];
    if (m_selectedIndex == 0)
    {
        std::snprintf(rowLabel, sizeof(rowLabel), "RESOLUTION  <  %s  >",
            RESOLUTIONS[m_pendingResolutionIndex].label);
    }
    else
    {
        std::snprintf(rowLabel, sizeof(rowLabel), "RESOLUTION     %s",
            RESOLUTIONS[m_pendingResolutionIndex].label);
    }
    DrawSciFiButton(rowLabel, m_selectedIndex == 0, 440.0f, centerX);

    ImGui::Spacing();

    DrawSciFiButton("APPLY", m_selectedIndex == 1, 280.0f, centerX);
    DrawSciFiButton("BACK", m_selectedIndex == 2, 280.0f, centerX);

    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.3f);
    const char* hint = "[W/S] Row  [A/D] Cycle  [ENTER] Confirm  [ESC] Back";
    const float hintWidth = ImGui::CalcTextSize(hint).x;
    ImGui::SetCursorPosX((windowWidth - hintWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), hint);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
}
