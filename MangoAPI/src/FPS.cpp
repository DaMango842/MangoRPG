#include "FPS.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

FPS::FPS(const sf::Font& font, float sampleInterval)
    : m_sampleInterval(sampleInterval)
{
    m_text.setFont(font);
    m_text.setCharacterSize(14);
    m_text.setFillColor(sf::Color::White);
    m_text.setPosition(10.f, 10.f);

    m_background.setFillColor(sf::Color(0, 0, 0, 150));
    m_background.setSize(sf::Vector2f(160, 60));
}

void FPS::update(float deltaTime)
{
    if (!m_visible) return;

    // FPS统计
    m_timeAccumulator += deltaTime;
    m_frameCounter++;

    if (m_timeAccumulator >= m_sampleInterval)
    {
        float fps = static_cast<float>(m_frameCounter) / m_timeAccumulator;
        m_currentFPS = fps;

        if (fps < m_minFPS) m_minFPS = fps;
        if (fps > m_maxFPS) m_maxFPS = fps;

        updateText();

        m_timeAccumulator = 0.f;
        m_frameCounter = 0;
    }

    // 内存信息更新（限频）
    m_memTimeAccumulator += deltaTime;
    if (m_memTimeAccumulator >= m_memUpdateInterval) {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            m_cachedMemMB = static_cast<size_t>(pmc.WorkingSetSize / (static_cast<unsigned long long>(1024) * 1024));
        }
#endif
        m_memTimeAccumulator = 0.f;
    }
}

void FPS::render(sf::RenderTarget& target, float x, float y)
{
    if (!m_visible) return;

    m_text.setPosition(x + 8.f, y + 6.f);
    m_background.setPosition(x, y);

    target.draw(m_background);
    target.draw(m_text);
}

bool FPS::toggleVisible()
{
    m_visible = !m_visible;
    return m_visible;
}

void FPS::updateText()
{
    int fpsInt = static_cast<int>(m_currentFPS + 0.5f);

    if (fpsInt == m_lastFpsInt && !m_cachedString.empty())
        return;

    m_lastFpsInt = fpsInt;

    int minInt = static_cast<int>(m_minFPS + 0.5f);
    int maxInt = static_cast<int>(m_maxFPS + 0.5f);

    // 组装字符串（只在 FPS 变化时更新）
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
        "[FPS] %d  Min: %d  Max: %d\n[MEM] %zu MB",
        fpsInt, minInt, maxInt, m_cachedMemMB);

    m_cachedString = buffer;
    m_text.setString(m_cachedString);

    // 根据 FPS 设置颜色
    if (fpsInt >= 60)
        m_text.setFillColor(sf::Color::Green);
    else if (fpsInt >= 30)
        m_text.setFillColor(sf::Color::Yellow);
    else
        m_text.setFillColor(sf::Color::Red);
}
