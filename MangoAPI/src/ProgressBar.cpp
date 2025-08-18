#include "ProgressBar.h"
#include "ResourceLoader.h"

ProgressBar::ProgressBar(float min, float max, float current, const sf::Vector2f& size)
    : m_min(min), m_max(max), m_currentValue(current), m_displayedValue(current), m_size(size),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{
    m_bgRect.setSize(size);
    m_bgRect.setFillColor(m_backgroundColor);

    m_fgRect.setFillColor(m_colorFull);

    m_text.setFont(m_font);
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color::White);
    m_text.setOutlineColor(sf::Color::Black);
    m_text.setOutlineThickness(1.5f);

    updateVisual();
}

void ProgressBar::setValue(float value)
{
    m_currentValue = std::clamp(value, m_min, m_max);
}

void ProgressBar::setInstantValue(float value)
{
    m_currentValue = m_displayedValue = std::clamp(value, m_min, m_max);
    updateVisual();
}

void ProgressBar::setRange(float min, float max)
{
    m_min = min;
    m_max = max;
    updateVisual();
}

void ProgressBar::setSize(const sf::Vector2f& size)
{
    m_size = size;
    m_bgRect.setSize(size);
    updateVisual();
}

void ProgressBar::setPosition(const sf::Vector2f& pos)
{
    m_position = pos;
    m_bgRect.setPosition(pos);
    updateVisual();
}

void ProgressBar::setBarColors(
    std::optional<sf::Color> full,
    std::optional<sf::Color> mid,
    std::optional<sf::Color> low)
{
    if (full.has_value()) m_colorFull = full.value();
    if (mid.has_value())  m_colorMid = mid.value();
    if (low.has_value())  m_colorLow = low.value();
}

void ProgressBar::setBackgroundColor(const sf::Color& color)
{
    m_backgroundColor = color;
    m_bgRect.setFillColor(color);
}

void ProgressBar::showText(bool enable)
{
    m_showText = enable;
}

void ProgressBar::handleEvent(const sf::Event& event)
{
    // No interaction
}

void ProgressBar::update(float deltaTime)
{
    /*if (m_displayedValue != m_currentValue) {
        float diff = m_currentValue - m_displayedValue;
        float step = diff * deltaTime * m_lerpSpeed;

        if (std::abs(step) < 0.01f)
            m_displayedValue = m_currentValue;
        else
            m_displayedValue += step;

        updateVisual();
    }*/
    if (std::abs(m_displayedValue - m_currentValue) > 0.1f) {
        m_displayedValue += (m_currentValue - m_displayedValue) * std::min(1.f, deltaTime * m_lerpSpeed);
        updateVisual();
    }
    else {
        m_displayedValue = m_currentValue;
    }
}

void ProgressBar::render(sf::RenderTarget& target)
{
    target.draw(m_bgRect);
    target.draw(m_fgRect);
    if (m_showText)
        target.draw(m_text);
}

void ProgressBar::updateVisual()
{
    float ratio = (m_displayedValue - m_min) / (m_max - m_min);
    ratio = std::clamp(ratio, 0.f, 1.f);

    sf::Vector2f barSize{ m_size.x * ratio, m_size.y };
    m_fgRect.setSize(barSize);
    m_fgRect.setPosition(m_position);
    m_fgRect.setFillColor(getInterpolatedColor(ratio));

    if (m_showText) {
        m_text.setString(std::format("{:.0f} / {:.0f}", m_displayedValue, m_max));
        sf::FloatRect textBounds = m_text.getLocalBounds();
        m_text.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
        m_text.setPosition(m_position.x + m_size.x / 2.f, m_position.y + m_size.y / 2.f - 2.f);
    }
}

sf::Color ProgressBar::getInterpolatedColor(float ratio)
{
    if (ratio > 0.5f)
        return m_colorFull;
    else if (ratio > 0.25f)
        return m_colorMid;
    else
        return m_colorLow;
}
