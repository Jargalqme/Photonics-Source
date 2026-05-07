#pragma once

#include "Scenes/Scene.h"
#include "Services/AudioManager.h"
#include "Scenes/MenuBackground.h"

class SceneManager;

class MainMenuScene : public Scene
{
public:
    MainMenuScene(SceneManager* sceneManager);
    ~MainMenuScene() override;

    void initialize(SceneContext& context) override;
    void enter() override;
    void exit() override;
    void finalize() override;

    void update(float deltaTime, InputManager* input) override;
    void render() override;

private:
    enum class State { Root, Settings };

    SceneManager* m_sceneManager;
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<MenuBackground> m_background;

    State m_state = State::Root;
    int m_selectedIndex = 0;

    static constexpr int ROOT_ITEM_COUNT = 4;      // Boss Fight, Training, Settings, Quit
    static constexpr int SETTINGS_ITEM_COUNT = 3;  // Resolution, Apply, Back

    int m_pendingResolutionIndex = 0;

    void updateRoot(InputManager* input);
    void updateSettings(InputManager* input);
    void renderRoot();
    void renderSettings();
    void enterSettings();
    void applySettings();
};
