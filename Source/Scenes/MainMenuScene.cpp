#include "pch.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/SceneManager.h"
#include "Services/InputManager.h"
#include "Renderer.h"
#include "Game.h"

#include "Render/Skinning/SkinnedModelImporter.h"
#include "Render/Skinning/SkinnedModel.h"
#include "Render/Skinning/SkinnedRenderer.h"
#include "Render/Skinning/Skeleton.h"
#include "Render/Skinning/AnimationPlayer.h"
#include "DeviceResources.h"

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
    constexpr ImGuiWindowFlags MENU_WINDOW_FLAGS =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    void DrawCenteredScaledText(const char* text, const ImVec4& color, float desiredFontSize, float maxWidth)
    {
        ImFont* font = ImGui::GetFont();
        float fontSize = desiredFontSize;
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        if (textSize.x > maxWidth && textSize.x > 0.0f)
        {
            fontSize *= maxWidth / textSize.x;
            textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        }

        const ImVec2 cursor = ImGui::GetCursorScreenPos();
        const float x = cursor.x + (ImGui::GetWindowWidth() - textSize.x) * 0.5f;
        ImGui::GetWindowDrawList()->AddText(
            font,
            fontSize,
            ImVec2(x, cursor.y),
            ImGui::ColorConvertFloat4ToU32(color),
            text);
        ImGui::Dummy(ImVec2(ImGui::GetWindowWidth(), textSize.y));
    }

    bool DrawSciFiButton(
        const char* text,
        bool isSelected,
        float width,
        float centerX,
        bool* outHovered = nullptr,
        float desiredTextSize = 32.0f)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        const float height = 44.0f;
        const ImVec2 originalCursor = ImGui::GetCursorScreenPos();
        const float x = centerX - width / 2.0f;
        const float y = originalCursor.y;

        ImGui::SetCursorScreenPos(ImVec2(x, y));
        ImGui::InvisibleButton(text, ImVec2(width, height));

        const bool isHovered = ImGui::IsItemHovered();
        const bool isClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        const bool isActive = isSelected || isHovered;
        if (outHovered)
        {
            *outHovered = isHovered;
        }

        const ImU32 bgColor = isActive ? IM_COL32(0, 60, 80, 220) : IM_COL32(0, 20, 30, 200);
        const ImU32 borderColor = isActive ? IM_COL32(0, 255, 255, 255) : IM_COL32(0, 150, 150, 180);
        const ImU32 glowColor = IM_COL32(0, 255, 255, isActive ? 60 : 0);
        const ImU32 textColor = isActive ? IM_COL32(0, 255, 255, 255) : IM_COL32(150, 200, 200, 255);

        if (isActive)
        {
            drawList->AddRectFilled(
                ImVec2(x - 4, y - 4),
                ImVec2(x + width + 4, y + height + 4),
                glowColor,
                6.0f);
        }

        drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), bgColor, 4.0f);
        drawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), borderColor, 4.0f, 0, isActive ? 2.0f : 1.0f);

        const float cornerSize = 8.0f;
        drawList->AddLine(ImVec2(x, y + cornerSize), ImVec2(x, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y), ImVec2(x + cornerSize, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width - cornerSize, y), ImVec2(x + width, y), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + cornerSize), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y + height - cornerSize), ImVec2(x, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x, y + height), ImVec2(x + cornerSize, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width - cornerSize, y + height), ImVec2(x + width, y + height), borderColor, 2.0f);
        drawList->AddLine(ImVec2(x + width, y + height - cornerSize), ImVec2(x + width, y + height), borderColor, 2.0f);

        ImFont* font = ImGui::GetFont();
        float fontSize = desiredTextSize;
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        const float maxTextWidth = width - 28.0f;
        if (textSize.x > maxTextWidth && textSize.x > 0.0f)
        {
            fontSize *= maxTextWidth / textSize.x;
            textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        }

        const float textX = x + (width - textSize.x) / 2.0f;
        const float textY = y + (height - textSize.y) / 2.0f;
        drawList->AddText(font, fontSize, ImVec2(textX, textY), textColor, text);

        ImGui::SetCursorScreenPos(ImVec2(originalCursor.x, y + height));
        ImGui::Dummy(ImVec2(1.0f, 12.0f));
        return isClicked;
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

    {
        SkinnedModelData data;
        const auto modelPath = GetAssetPath(L"Characters/MenuGuy/model.fbx");
        if (SkinnedModelImporter::LoadSkinnedModelData(modelPath, data))
        {
            size_t selectedClipIndex = data.clips.empty() ? 0 : data.clips.size() - 1;
            bool hasSelectedClip = !data.clips.empty();
            const size_t firstAppendedClipIndex = data.clips.size();

            const auto dancePath = GetAssetPath(L"Characters/MenuGuy/dance.fbx");
            const int32_t appended =
                SkinnedModelImporter::AppendClipsFromFile(dancePath, data);
            if (appended > 0)
            {
                selectedClipIndex = firstAppendedClipIndex;
                hasSelectedClip = true;
            }

            m_menuCharacter = std::make_unique<SkinnedModel>();
            if (!m_menuCharacter->initialize(
                    m_deviceResources->GetD3DDevice(), std::move(data)))
            {
                m_menuCharacter.reset();
            }

            if (m_menuCharacter)
            {
                m_skinnedRenderer = std::make_unique<SkinnedRenderer>();
                if (!m_skinnedRenderer->initialize(m_deviceResources->GetD3DDevice()))
                {
                    m_skinnedRenderer.reset();
                }

                m_skeleton = std::make_unique<Skeleton>();
                m_skeleton->build(m_menuCharacter->data());

                m_animationPlayer = std::make_unique<AnimationPlayer>();
                const auto& clips = m_menuCharacter->clips();
                if (hasSelectedClip && selectedClipIndex < clips.size())
                {
                    m_animationPlayer->setClip(&clips[selectedClipIndex]);
                    m_animationPlayer->setLoop(true);
                }
                else if (!clips.empty())
                {
                    m_animationPlayer->setClip(&clips.back());
                    m_animationPlayer->setLoop(true);
                }
            }
        }
    }

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
    if (m_context && m_context->input)
    {
        m_context->input->setCursorVisible(true);
    }

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

    if (m_animationPlayer)
    {
        m_animationPlayer->update(deltaTime);
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

    if (m_menuCharacter && m_skinnedRenderer)
    {
        const int rw = m_renderer->GetRenderWidth();
        const int rh = m_renderer->GetRenderHeight();
        const float aspect = (rh > 0) ? (static_cast<float>(rw) / static_cast<float>(rh)) : 1.0f;

        const Vector3 cameraPos(0.0f, 0.0f, 0.0f);
        const Vector3 cameraTarget(0.0f, 0.0f, -400.0f);

        const Vector3 characterOffset(-120.0f, -80.0f, -400.0f);
        const float characterYaw = XM_PI * 1.1f;

        const Matrix view = Matrix::CreateLookAt(cameraPos, cameraTarget, Vector3::Up);
        const Matrix proj = Matrix::CreatePerspectiveFieldOfView(
            XMConvertToRadians(35.0f), aspect, 1.0f, 5000.0f);
        const Matrix world =
            Matrix::CreateRotationY(characterYaw) *
            Matrix::CreateTranslation(characterOffset);

        // Sample the dance clip into the skeleton, build the bone palette,
        // and hand it to the renderer.
        const DirectX::SimpleMath::Matrix* palette = nullptr;
        uint32_t paletteCount = 0;
        if (m_skeleton && m_animationPlayer)
        {
            m_animationPlayer->apply(*m_skeleton);
            palette      = m_skeleton->palette();
            paletteCount = m_skeleton->boneCount();
        }

        //m_skinnedRenderer->draw(
        //    m_deviceResources->GetD3DDeviceContext(),
        //    *m_menuCharacter,
        //    palette, paletteCount,
        //    world, view, proj);

        const Vector4 menuLight(0.0f, -1.0f, 0.0f, 0.35f);

        m_skinnedRenderer->draw(
            m_deviceResources->GetD3DDeviceContext(),
            *m_menuCharacter,
            palette, paletteCount,
            world, view, proj,
            Vector4(0.9f, 0.9f, 0.9f, 1.0f),
            menuLight);
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

    ImGui::Begin("##MainMenu", nullptr, MENU_WINDOW_FLAGS);

    const float windowWidth = ImGui::GetWindowWidth();

    DrawCenteredScaledText(
        "P H O T O N X",
        ImVec4(0.0f, 1.0f, 1.0f, 1.0f),
        80.0f,
        windowWidth - 24.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    const char* menuItems[ROOT_ITEM_COUNT] = { "START A GAME", "WORKSHOP", "SETTINGS", "QUIT" };
    for (int i = 0; i < ROOT_ITEM_COUNT; i++)
    {
        bool hovered = false;
        if (DrawSciFiButton(menuItems[i], i == m_selectedIndex, 280.0f, centerX, &hovered))
        {
            m_selectedIndex = i;
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
        else if (hovered)
        {
            m_selectedIndex = i;
        }
    }

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

    ImGui::Begin("##MainMenuSettings", nullptr, MENU_WINDOW_FLAGS);

    const float windowWidth = ImGui::GetWindowWidth();

    DrawCenteredScaledText(
        "SETTINGS",
        ImVec4(0.0f, 1.0f, 1.0f, 1.0f),
        80.0f,
        windowWidth - 24.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

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
    bool hovered = false;
    if (DrawSciFiButton(rowLabel, m_selectedIndex == 0, 440.0f, centerX, &hovered))
    {
        m_selectedIndex = 0;
        m_pendingResolutionIndex = (m_pendingResolutionIndex + 1) % RESOLUTION_COUNT;
    }
    else if (hovered)
    {
        m_selectedIndex = 0;
    }

    ImGui::Spacing();

    hovered = false;
    if (DrawSciFiButton("APPLY", m_selectedIndex == 1, 280.0f, centerX, &hovered))
    {
        m_selectedIndex = 1;
        applySettings();
    }
    else if (hovered)
    {
        m_selectedIndex = 1;
    }

    hovered = false;
    if (DrawSciFiButton("BACK", m_selectedIndex == 2, 280.0f, centerX, &hovered))
    {
        m_state = State::Root;
        m_selectedIndex = 2;
    }
    else if (hovered)
    {
        m_selectedIndex = 2;
    }

    ImGui::End();
}
