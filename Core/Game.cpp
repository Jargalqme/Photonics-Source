#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_input = std::make_unique<InputManager>();
    m_renderer = std::make_unique<Renderer>(m_deviceResources.get());
    m_sceneManager = std::make_unique<SceneManager>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();
    m_renderer->CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
    m_renderer->CreateWindowSizeDependentResources();

    m_input->initialize(window);
    m_sceneManager->Initialize(m_deviceResources.get());

    // Create and add scenes
    auto introScene = std::make_unique<IntroScene>(m_sceneManager.get());
    m_sceneManager->AddScene("Intro", std::move(introScene));

    auto mainMenuScene = std::make_unique<MainMenuScene>(m_sceneManager.get());
    m_sceneManager->AddScene("MainMenu", std::move(mainMenuScene));

    auto gameScene = std::make_unique<GameScene>(m_sceneManager.get());
    m_sceneManager->AddScene("GameScene", std::move(gameScene));

    auto victoryScene = std::make_unique<VictoryScene>(m_sceneManager.get());
    m_sceneManager->AddScene("Victory", std::move(victoryScene));

    auto gameOverScene = std::make_unique<GameOverScene>(m_sceneManager.get());
    m_sceneManager->AddScene("GameOver", std::move(gameOverScene));

    // Start with main menu
    m_sceneManager->TransitionTo("MainMenu");

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Disable imgui.ini for distribution

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });
    Render(); // Call the Game::Render() method
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float deltaTime = float(timer.GetElapsedSeconds());

    // Update input first!
    m_input->update();

    // Update active scene
    m_sceneManager->Update(deltaTime, m_input.get());
}

#pragma endregion

#pragma region Frame Render
void Game::Render()
{
    // === BEGIN SCENE (sets up render target) ===
    m_renderer->BeginScene();

    // === IMGUI FRAME ===
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // === RENDER SCENE ===
    m_sceneManager->Render(m_renderer.get());

    // === END SCENE (copies to backbuffer) ===
    m_renderer->EndScene();

    // === IMGUI RENDER ===
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // === PRESENT ===
    m_deviceResources->Present();
}


#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    m_renderer->CreateWindowSizeDependentResources();

    m_sceneManager->OnWindowSizeChanged(width, height);
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1920;
    height = 1080;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    // Device-dependent resources are now created in the Renderer
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    ImGui_ImplDX11_InvalidateDeviceObjects();
    m_renderer->OnDeviceLost();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    m_renderer->CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
    m_renderer->CreateWindowSizeDependentResources();

    ImGui_ImplDX11_CreateDeviceObjects();
}
#pragma endregion
