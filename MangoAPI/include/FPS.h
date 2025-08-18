#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <limits>

class FPS
{
public:
    explicit FPS(const sf::Font& font, float sampleInterval = 0.5f);

    void update(float deltaTime);
    void render(sf::RenderTarget& target, float x = 10.f, float y = 10.f);

    bool toggleVisible();

private:
    sf::Text m_text;
    sf::RectangleShape m_background;

    float m_sampleInterval;
    float m_timeAccumulator = 0.f;
    int m_frameCounter = 0;

    float m_currentFPS = -1.f;
    float m_minFPS = std::numeric_limits<float>::max();
    float m_maxFPS = 0.f;

    bool m_visible = true;

    std::string m_cachedString;
    int m_lastFpsInt = -1;

    // 内存统计相关
    float m_memUpdateInterval = 1.f; // 每秒更新一次内存
    float m_memTimeAccumulator = 0.f;
    size_t m_cachedMemMB = 0;

    void updateText();
};
