#pragma once
#include "Scene.h"
#include "MenuBackground.h"

class SceneManager;

class IntroScene : public Scene
{
public:
    IntroScene(SceneManager* sceneManager);
    ~IntroScene() override;

    void Initialize(DX::DeviceResources* deviceResources) override;
    void Enter() override;
    void Exit() override;
    void Cleanup() override;

    void Update(float deltaTime, InputManager* input) override;
    void Render(Renderer* renderer) override;

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    SceneManager* m_sceneManager;
    std::unique_ptr<MenuBackground> m_background;

    // Intro state machine
    int m_introState = 0;
    float m_introTimer = 0.0f;
    float m_quoteAlpha = 0.0f;

    // Timing constants
    static constexpr float WAIT_TIME = 3.0f;
    static constexpr float QUOTE_FADE_IN_TIME = 1.0f;
    static constexpr float QUOTE_HOLD_TIME = 3.0f;
    static constexpr float QUOTE_FADE_OUT_TIME = 1.0f;

    const wchar_t* m_quoteText = L"The Battle Between Light and Darkness";
};
