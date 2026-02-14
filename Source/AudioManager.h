#pragma once
// ============================================================================
// AudioManager.h - Simple DirectXTK Audio Wrapper
// What it does:
//  - Plays sound effects (fire and forget)
//	- Plays background music (with loop)
//  - Volume control
//  - Safe cleanup
// ============================================================================

#include "Audio.h"           // DirectXTK`s audio wrapper (AudioEngine, SoundEffect, etc.)
#include <string>            // For sound names ("beam", "music")
#include <unordered_map>     // Hash table for fast lookup by name
#include <memory>            // std::unique_ptr for automatic cleanup

class AudioManager
{
public:
	AudioManager();
	~AudioManager();

	// No copying (audio system should be unique)
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	// === Lifecycle ===
	bool initialize();
	void update();
	void finalize();

	// === Loading ===
	bool loadSound(const std::string& name, const std::wstring& filepath);
	bool loadMusic(const std::string& name, const std::wstring& filepath);

	// === Playback ===
	void playSound(const std::string& name, float volume = 1.0f);
	void playMusic(const std::string& name, bool loop = true);
	void stopMusic();

	// === Volume ===
	void setMasterVolume(float volume);
	float getMasterVolume() const { return m_masterVolume; }
	float* getMasterVolumePtr() { return &m_masterVolume; }

	// === State ===
	bool isInitialized() const { return m_initialized; }

private:
	// Engine (destroyed LAST)
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

	// Sound storage (destroyed SECOND)
	std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffect>> m_sounds;
	std::unordered_map<std::string, std::unique_ptr<DirectX::SoundEffect>> m_music;

	// Active music instance (destroyed FIRST)
	std::unique_ptr<DirectX::SoundEffectInstance> m_currentMusic;
	std::string m_currentMusicName;

	// State
	float m_masterVolume = 0.8f; // was 1.0
	bool m_initialized = false;
};
