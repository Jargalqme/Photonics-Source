#pragma once

#include "DeviceResources.h"     
#include "StepTimer.h"          
#include "Renderer.h"      
#include "Source/InputManager.h" 
#include "Source/SceneManager.h" 
#include "Source/GameScene.h"   
#include "Source/IntroScene.h"
#include "Source/MainMenuScene.h"
#include "Source/VictoryScene.h"
#include "Source/GameOverScene.h"
#include <memory>

class Game final : public DX::IDeviceNotify  
{
public: 

    Game() noexcept(false);
    ~Game();  

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    void Initialize(HWND window, int width, int height);

    void Tick();

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    void GetDefaultSize(int& width, int& height) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    DX::StepTimer                           m_timer;

    std::unique_ptr<Renderer>               m_renderer;

    std::unique_ptr<InputManager>           m_input;

    std::unique_ptr<SceneManager>           m_sceneManager;
};