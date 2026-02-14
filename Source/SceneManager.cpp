#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Renderer.h"
#include <cassert>

SceneManager::SceneManager()
	: m_deviceResources(nullptr)
	, m_activeScene(nullptr)
{
}

SceneManager::~SceneManager()
{
	Cleanup();
}

void SceneManager::Initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;
}

void SceneManager::AddScene(const std::string& name, std::unique_ptr<Scene> scene)
{
	assert(scene != nullptr && "Cannot add null scene");

	// Initialize the scene
	scene->Initialize(m_deviceResources);

	// Add to collection
	m_scenes[name] = std::move(scene);
}

void SceneManager::RemoveScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		if (m_activeScene == it->second.get())
		{
			m_activeScene->Exit();
			m_activeScene = nullptr;
		}

		it->second->Cleanup();
		m_scenes.erase(it);
	}
}

void SceneManager::SetActiveScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		// Exit current scene
		if (m_activeScene)
		{
			m_activeScene->Exit();
		}

		// Clear the stack when setting a new scene directly
		while (!m_sceneStack.empty())
		{
			m_sceneStack.pop();
		}

		// Enter new scene
		m_activeScene = it->second.get();
		m_activeScene->Enter();
	}
}

void SceneManager::PushScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		// Pause current scene (exit but keep on stack)
		if (m_activeScene)
		{
			m_activeScene->Exit();
			m_sceneStack.push(m_activeScene);
		}

		// Enter new scene
		m_activeScene = it->second.get();
		m_activeScene->Enter();
	}
}

void SceneManager::PopScene()
{
	if (!m_sceneStack.empty())
	{
		// Exit current scene
		if (m_activeScene)
		{
			m_activeScene->Exit();
		}

		// Return to previous scene
		m_activeScene = m_sceneStack.top();
		m_sceneStack.pop();

		if (m_activeScene)
		{
			m_activeScene->Enter();
		}
	}
}

void SceneManager::TransitionTo(const std::string& sceneName, float duration)
{
	// Don't start new transition if already transitioning
	if (m_fadingOut || m_fadingIn)
		return;

	// Verify scene exists
	if (!HasScene(sceneName))
		return;

	m_pendingScene = sceneName;
	m_fadeDuration = duration;
	m_fadingOut = true;
	m_fadeAlpha = 0.0f;
}

void SceneManager::UpdateTransition(float deltaTime)
{
	if (m_fadingOut)
	{
		// Fade to black
		m_fadeAlpha += deltaTime / m_fadeDuration;
		if (m_fadeAlpha >= 1.0f)
		{
			m_fadeAlpha = 1.0f;

			// Switch scene while fully black
			SetActiveScene(m_pendingScene);
			m_pendingScene.clear();

			// Start fading back in
			m_fadingOut = false;
			m_fadingIn = true;
		}
	}
	else if (m_fadingIn)
	{
		// Fade from black
		m_fadeAlpha -= deltaTime / m_fadeDuration;
		if (m_fadeAlpha <= 0.0f)
		{
			m_fadeAlpha = 0.0f;
			m_fadingIn = false;
		}
	}
}

void SceneManager::Update(float deltaTime, InputManager* input)
{
	UpdateTransition(deltaTime);

	if (m_activeScene && m_activeScene->IsActive())
	{
		m_activeScene->Update(deltaTime, input);
	}
}

void SceneManager::Render(Renderer* renderer)
{
	if (m_activeScene)
	{
		m_activeScene->Render(renderer);
	}

	// Fade overlay - Must wrap in BeginUI/EndUI
	if (m_fadeAlpha > 0.0f)
	{
		renderer->BeginUI();
		renderer->DrawRect(renderer->GetFullscreenRect(), DirectX::Colors::Black, m_fadeAlpha);
		renderer->EndUI();
	}
}

void SceneManager::OnWindowSizeChanged(int width, int height)
{
	if (m_activeScene)
	{
		m_activeScene->OnWindowSizeChanged(width, height);
	}
}

void SceneManager::OnDeviceLost()
{
	if (m_activeScene)
	{
		m_activeScene->OnDeviceLost();
	}
}

void SceneManager::OnDeviceRestored()
{
	if (m_activeScene)
	{
		m_activeScene->OnDeviceRestored();
	}
}

Scene* SceneManager::GetScene(const std::string& name) const
{
	auto it = m_scenes.find(name);
	return (it != m_scenes.end()) ? it->second.get() : nullptr;
}

bool SceneManager::HasScene(const std::string& name) const
{
	return m_scenes.find(name) != m_scenes.end();
}

void SceneManager::Cleanup()
{
	// Exit active scene
	if (m_activeScene)
	{
		m_activeScene->Exit();
		m_activeScene = nullptr;
	}

	// Cleanup all scenes
	for (auto& pair : m_scenes)
	{
		pair.second->Cleanup();
	}

	m_scenes.clear();

	while (!m_sceneStack.empty())
	{
		m_sceneStack.pop();
	}
}