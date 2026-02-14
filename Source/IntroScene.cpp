#include "pch.h"
#include "IntroScene.h"
#include "SceneManager.h"
#include "Renderer.h"

IntroScene::IntroScene(SceneManager* sceneManager)
    : Scene("Intro")
    , m_sceneManager(sceneManager)
{
}

IntroScene::~IntroScene()
{
}

void IntroScene::Initialize(DX::DeviceResources* deviceResources)
{
    m_deviceResources = deviceResources;

    m_background = std::make_unique<MenuBackground>(deviceResources);
    m_background->Initialize();
    m_background->SetShaderType(MenuBackground::ShaderType::Warping);
    m_background->SetSpeed(1.0f);
    m_background->SetPatternScale(9.0f);
    m_background->SetWarpIntensity(1.0f);
    m_background->SetBrightness(0.01f);
    m_background->SetChromaticOffset(0.07f);
}

void IntroScene::Enter()
{
    Scene::Enter();
    m_introState = 0;
    m_introTimer = 0.0f;
    m_quoteAlpha = 0.0f;
}

void IntroScene::Exit()
{
    Scene::Exit();
}

void IntroScene::Cleanup()
{
}

void IntroScene::Update(float deltaTime, InputManager* input)
{
    (void)input;

    if (m_background)
        m_background->Update(deltaTime);

    m_introTimer += deltaTime;

    switch (m_introState)
    {
    case 0:  // Wait (shader only visible)
        if (m_introTimer >= WAIT_TIME)
        {
            m_introState = 1;
            m_introTimer = 0.0f;
        }
        break;

    case 1:  // Fade in quote
        m_quoteAlpha = std::min(m_introTimer / QUOTE_FADE_IN_TIME, 1.0f);
        if (m_introTimer >= QUOTE_FADE_IN_TIME)
        {
            m_introState = 2;
            m_introTimer = 0.0f;
            m_quoteAlpha = 1.0f;
        }
        break;

    case 2:  // Hold quote
        if (m_introTimer >= QUOTE_HOLD_TIME)
        {
            m_introState = 3;
            m_introTimer = 0.0f;
        }
        break;

    case 3:  // Fade out quote
        m_quoteAlpha = std::max(1.0f - (m_introTimer / QUOTE_FADE_OUT_TIME), 0.0f);
        if (m_introTimer >= QUOTE_FADE_OUT_TIME)
        {
            m_sceneManager->TransitionTo("MainMenu");
        }
        break;
    }
}

void IntroScene::Render(Renderer* renderer)
{
    if (m_background)
        m_background->Render();

    if (m_quoteAlpha > 0.0f)
    {
        auto size = m_deviceResources->GetOutputSize();
        float centerX = (size.right - size.left) * 0.5f;
        float centerY = (size.bottom - size.top) * 0.9f; // lower

        renderer->BeginUI();
        renderer->DrawTextCentered(
            m_quoteText,
            XMFLOAT2(centerX, centerY),
            Colors::White,
            m_quoteAlpha,
            FontType::Quote
        );
        renderer->EndUI();
    }
}

void IntroScene::OnDeviceLost()
{
    if (m_background)
        m_background->OnDeviceLost();
}

void IntroScene::OnDeviceRestored()
{
    if (m_background)
    {
        m_background->Initialize();
        m_background->SetShaderType(MenuBackground::ShaderType::Warping);
    }
}