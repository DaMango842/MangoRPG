#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>

class MusicManager
{
public:
    MusicManager() = default;
    ~MusicManager() = default;

    // 加载音乐
    void load(const std::string& name, const std::string& filename)
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filename))
            throw std::runtime_error("Failed to load music: " + filename);

        m_musicMap[name] = std::move(music);
    }

    // 播放指定音乐
    void play(const std::string& name, bool loop = true)
    {
        stop();

        auto it = m_musicMap.find(name);
        if (it == m_musicMap.end())
            throw std::runtime_error("Music not found: " + name);

        m_current = it->second.get();
        m_current->setLoop(loop);
        m_current->play();
    }

    void stop()
    {
        if (m_current && m_current->getStatus() == sf::Music::Playing)
            m_current->stop();
        m_current = nullptr;
    }

    void pause()
    {
        if (m_current && m_current->getStatus() == sf::Music::Playing)
            m_current->pause();
    }

    void resume()
    {
        if (m_current && m_current->getStatus() == sf::Music::Paused)
            m_current->play();
    }

    void setVolume(float volume)
    {
        if (m_current)
            m_current->setVolume(volume);
    }

    float getVolume() const
    {
        return m_current ? m_current->getVolume() : 0.f;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> m_musicMap;
    sf::Music* m_current = nullptr;
};
