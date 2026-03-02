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

void IntroScene::initialize(DX::DeviceResources* deviceResources)
{
    Scene::initialize(deviceResources);

    m_background = std::make_unique<MenuBackground>(deviceResources);
    m_background->initialize();
    m_background->setShaderType(MenuBackground::ShaderType::Warping);
    m_background->setSpeed(1.0f);
    m_background->setPatternScale(9.0f);
    m_background->setWarpIntensity(1.0f);
    m_background->setBrightness(0.01f);
    m_background->setChromaticOffset(0.07f);
}

void IntroScene::enter()
{
    Scene::enter();
    m_introState = 0;
    m_introTimer = 0.0f;
    m_quoteAlpha = 0.0f;
}

void IntroScene::exit()
{
    Scene::exit();
}

void IntroScene::finalize()
{
    if (m_background) m_background->onDeviceLost();
}

void IntroScene::update(float deltaTime, InputManager* input)
{
    (void)input;

    if (m_background)
        m_background->update(deltaTime);

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
            m_sceneManager->transitionTo("MainMenu");
        }
        break;
    }
}

void IntroScene::render(Renderer* renderer)
{
    if (m_background)
        m_background->render();

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

