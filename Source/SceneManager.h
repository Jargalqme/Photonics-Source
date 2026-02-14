#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <stack>

class Scene;
class InputManager;
class Renderer;
namespace DX { class DeviceResources; }

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void Initialize(DX::DeviceResources* deviceResources);

    // Scene management
    void AddScene(const std::string& name, std::unique_ptr<Scene> scene);
    void RemoveScene(const std::string& name);
    void SetActiveScene(const std::string& name);
    void PushScene(const std::string& name);
    void PopScene();

    // Fade transition
    void TransitionTo(const std::string& sceneName, float duration = 0.5f);
    bool IsTransitioning() const { return m_fadingOut || m_fadingIn; }

    // Update and render
    void Update(float deltaTime, InputManager* input);
    void Render(Renderer* renderer);

    // Window events
    void OnWindowSizeChanged(int width, int height);
    void OnDeviceLost();
    void OnDeviceRestored();

    // Getters
    Scene* GetActiveScene() const { return m_activeScene; }
    Scene* GetScene(const std::string& name) const;
    bool HasScene(const std::string& name) const;

    void Cleanup();

private:
    DX::DeviceResources* m_deviceResources;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene;
    std::stack<Scene*> m_sceneStack;

    // === Fade Transition ===
    float m_fadeAlpha = 0.0f;
    float m_fadeDuration = 0.5f;
    bool m_fadingOut = false;
    bool m_fadingIn = false;
    std::string m_pendingScene;

    void UpdateTransition(float deltaTime);
};

