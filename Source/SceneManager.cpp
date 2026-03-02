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
	finalize();
}

void SceneManager::initialize(DX::DeviceResources* deviceResources)
{
	m_deviceResources = deviceResources;
}

void SceneManager::addScene(const std::string& name, std::unique_ptr<Scene> scene)
{
	assert(scene != nullptr && "Cannot add null scene");

	// Initialize the scene
	scene->initialize(m_deviceResources);

	// Add to collection
	m_scenes[name] = std::move(scene);
}

void SceneManager::removeScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		if (m_activeScene == it->second.get())
		{
			m_activeScene->exit();
			m_activeScene = nullptr;
		}

		it->second->finalize();
		m_scenes.erase(it);
	}
}

void SceneManager::setActiveScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		// Exit current scene
		if (m_activeScene)
		{
			m_activeScene->exit();
		}

		// Clear the stack when setting a new scene directly
		while (!m_sceneStack.empty())
		{
			m_sceneStack.pop();
		}

		// Enter new scene
		m_activeScene = it->second.get();
		m_activeScene->enter();
	}
}

void SceneManager::pushScene(const std::string& name)
{
	auto it = m_scenes.find(name);
	if (it != m_scenes.end())
	{
		// Pause current scene (exit but keep on stack)
		if (m_activeScene)
		{
			m_activeScene->exit();
			m_sceneStack.push(m_activeScene);
		}

		// Enter new scene
		m_activeScene = it->second.get();
		m_activeScene->enter();
	}
}

void SceneManager::popScene()
{
	if (!m_sceneStack.empty())
	{
		// Exit current scene
		if (m_activeScene)
		{
			m_activeScene->exit();
		}

		// Return to previous scene
		m_activeScene = m_sceneStack.top();
		m_sceneStack.pop();

		if (m_activeScene)
		{
			m_activeScene->enter();
		}
	}
}

void SceneManager::transitionTo(const std::string& sceneName, float duration)
{
	// Don't start new transition if already transitioning
	if (m_fadingOut || m_fadingIn)
		return;

	// Verify scene exists
	if (!hasScene(sceneName))
		return;

	m_pendingScene = sceneName;
	m_fadeDuration = duration;
	m_fadingOut = true;
	m_fadeAlpha = 0.0f;
}

void SceneManager::updateTransition(float deltaTime)
{
	if (m_fadingOut)
	{
		// Fade to black
		m_fadeAlpha += deltaTime / m_fadeDuration;
		if (m_fadeAlpha >= 1.0f)
		{
			m_fadeAlpha = 1.0f;

			// Switch scene while fully black
			setActiveScene(m_pendingScene);
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

void SceneManager::update(float deltaTime, InputManager* input)
{
	updateTransition(deltaTime);

	if (m_activeScene && m_activeScene->isActive())
	{
		m_activeScene->update(deltaTime, input);
	}
}

void SceneManager::render(Renderer* renderer)
{
	if (m_activeScene)
	{
		m_activeScene->render(renderer);
	}

	// Fade overlay - Must wrap in BeginUI/EndUI
	if (m_fadeAlpha > 0.0f)
	{
		renderer->BeginUI();
		renderer->DrawRect(renderer->GetFullscreenRect(), DirectX::Colors::Black, m_fadeAlpha);
		renderer->EndUI();
	}
}

Scene* SceneManager::getScene(const std::string& name) const
{
	auto it = m_scenes.find(name);
	return (it != m_scenes.end()) ? it->second.get() : nullptr;
}

bool SceneManager::hasScene(const std::string& name) const
{
	return m_scenes.find(name) != m_scenes.end();
}

void SceneManager::finalize()
{
	// Exit active scene
	if (m_activeScene)
	{
		m_activeScene->exit();
		m_activeScene = nullptr;
	}

	// Finalize all scenes
	for (auto& pair : m_scenes)
	{
		pair.second->finalize();
	}

	m_scenes.clear();

	while (!m_sceneStack.empty())
	{
		m_sceneStack.pop();
	}
}
