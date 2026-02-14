#include "pch.h"
#include "AudioManager.h"

using namespace DirectX;

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
	finalize();
}

bool AudioManager::initialize()
{
	if (m_initialized)
		return true; // already init

	// Create audio engine with default flags
	AUDIO_ENGINE_FLAGS flags = AudioEngine_Default;

#ifdef _DEBUG
	flags |= AudioEngine_Debug; // Extra debug output in debug builds
#endif

	try
	{
		m_audioEngine = std::make_unique<AudioEngine>(flags);
	}
	catch (const std::exception& e)
	{
		// No audio device - game can still run silent
		OutputDebugStringA("AudioManager: Failed to create engine - ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
		return false;
	}

	m_initialized = true;
	OutputDebugStringA("AudioManager: Initialized successfully\n");
	return true;
}

void AudioManager::update()
{
	if (!m_audioEngine)
		return;

	// Check for device loss (headphones unplugged, etc.)
	if (m_audioEngine->IsCriticalError())
	{
		OutputDebugStringA("AudioManager: Critical error, attempting reset\n");
	}

	// Required: Process audio each frame
	m_audioEngine->Update();
}

void AudioManager::finalize()
{
	if (!m_initialized)
		return;

	// Stop music first
	if (m_currentMusic)
	{
		m_currentMusic->Stop();
		m_currentMusic.reset();
	}

	// Clear sounds (releases SoundEffect objects)
	m_sounds.clear();
	m_music.clear();

	// Reset engine (releases XAudio2)
	m_audioEngine.reset();

	m_initialized = false;
	OutputDebugStringA("AudioManager: Shutdown complete\n");
}

bool AudioManager::loadSound(const std::string& name, const std::wstring& filepath)
{
	if (!m_audioEngine)
		return false;

	try
	{
		m_sounds[name] = std::make_unique<SoundEffect>(m_audioEngine.get(), filepath.c_str());
		return true;
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("AudioManager: Failed to load sound - ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
		return false;
	}
}

bool AudioManager::loadMusic(const std::string& name, const std::wstring& filepath)
{
	if (!m_audioEngine)
		return false;

	try
	{
		m_music[name] = std::make_unique<SoundEffect>(m_audioEngine.get(), filepath.c_str());
		return true;
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("AudioManager: Failed to load music - ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
		return false;
	}
}

void AudioManager::playSound(const std::string& name, float volume)
{
	if (!m_audioEngine)
		return;

	auto it = m_sounds.find(name);
	if (it == m_sounds.end())
	{
		OutputDebugStringA("AudioManager: Sound not found - ");
		OutputDebugStringA(name.c_str());
		OutputDebugStringA("\n");
		return;
	}

	// Fire and Forget - DirectXTK manages the instance internally
	it->second->Play(volume * m_masterVolume, 0.0f, 0.0f);
}

void AudioManager::playMusic(const std::string& name, bool loop)
{
	if (!m_audioEngine)
		return;

	auto it = m_music.find(name);
	if (it == m_music.end())
	{
		OutputDebugStringA("AudioManager: Music not found - ");
		OutputDebugStringA(name.c_str());
		OutputDebugStringA("\n");
		return;
	}

	// Stop current music if playing
	stopMusic();

	// Create instance for control
	m_currentMusic = it->second->CreateInstance();
	m_currentMusicName = name;

	// Set volume and play
	m_currentMusic->SetVolume(m_masterVolume);
	m_currentMusic->Play(loop);
}

void AudioManager::stopMusic()
{
	if (m_currentMusic)
	{
		m_currentMusic->Stop();
		m_currentMusic.reset();
		m_currentMusicName.clear();
	}
}

void AudioManager::setMasterVolume(float volume)
{
	// Clamp to valid range
	m_masterVolume = std::max(0.0f, std::min(1.0f, volume));

	// Apply to current music
	if (m_currentMusic)
	{
		m_currentMusic->SetVolume(m_masterVolume);
	}
}