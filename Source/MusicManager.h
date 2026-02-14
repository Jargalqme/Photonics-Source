#pragma once
#include <functional>

class MusicManager
{
public:
    using BeatCallback = std::function<void(int beat)>;

    MusicManager() = default;

    void update(float deltaTime);
    void reset();

    void setBPM(float bpm);
    void setSongDuration(float seconds) { m_songDuration = seconds; }
    void setStartDelay(float seconds) { m_startDelay = seconds; }
    void setBeatCallback(BeatCallback callback) { m_beatCallback = callback; }

    float getBPM() const { return m_bpm; }
    float getSongDuration() const { return m_songDuration; }
    int getBeat() const { return m_beat; }
    float getBeatProgress() const;
    float getElapsedTime() const { return m_elapsedTime; }
    bool isSongComplete() const;
    bool isOnBeat(float threshold = 0.1f) const;

private:
    void onBeat();

    float m_bpm = 120.0f;
    float m_songDuration = 0.0f;
    float m_startDelay = 0.0f;
    bool m_active = false;

    float m_timeSinceLastBeat = 0.0f;
    float m_beatInterval = 0.5f;
    float m_elapsedTime = 0.0f;
    int m_beat = 1;
    BeatCallback m_beatCallback;
};