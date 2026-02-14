#include "pch.h"
#include "MusicManager.h"

void MusicManager::update(float deltaTime)
{
    if (m_bpm <= 0.0f)
        return;

    m_elapsedTime += deltaTime;

    // Don't count beats until start delay passed
    if (m_elapsedTime < m_startDelay)
        return;

    // Mark that we've started
    if (!m_active)
    {
        m_active = true;
        m_timeSinceLastBeat = 0.0f;
    }

    m_timeSinceLastBeat += deltaTime;

    while (m_timeSinceLastBeat >= m_beatInterval)
    {
        m_timeSinceLastBeat -= m_beatInterval;
        onBeat();
    }
}

void MusicManager::reset()
{
    m_timeSinceLastBeat = 0.0f;
    m_elapsedTime = 0.0f;
    m_beat = 1;
    m_active = false;
}

void MusicManager::setBPM(float bpm)
{
    if (bpm > 0.0f)
    {
        m_bpm = bpm;
        m_beatInterval = 60.0f / m_bpm;
    }
}

float MusicManager::getBeatProgress() const
{
    if (m_beatInterval <= 0.0f)
        return 0.0f;
    return m_timeSinceLastBeat / m_beatInterval;
}

bool MusicManager::isOnBeat(float threshold) const
{
    float timeToNext = m_beatInterval - m_timeSinceLastBeat;
    return (m_timeSinceLastBeat < threshold) || (timeToNext < threshold);
}

bool MusicManager::isSongComplete() const
{
    return m_songDuration > 0.0f && m_elapsedTime >= m_songDuration;
}

void MusicManager::onBeat()
{
    m_beat++;

    if (m_beatCallback)
        m_beatCallback(m_beat);
}
