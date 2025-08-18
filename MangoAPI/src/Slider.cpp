#include "Slider.h"
#include <cmath>

Slider::Slider(float min, float max, float step, sf::Font& font, Orientation orientation)
    : m_min(min), m_max(max), m_step(step), m_font(font), m_orientation(orientation), m_value(min)
{
    m_track.setFillColor(sf::Color(100, 100, 100));
    m_thumb.setFillColor(sf::Color(200, 200, 200));

    m_track.setSize({ 200.f, 4.f });
    m_thumb.setSize({ 10.f, 20.f });

    m_label.setFont(font);
    m_label.setCharacterSize(16);
    m_label.setFillColor(sf::Color::White);
    m_label.setString(L"滑动条");

    updateThumbPosition();
}

void Slider::setPosition(const sf::Vector2f& pos)
{
    m_track.setPosition(pos);
    m_label.setPosition(pos.x, pos.y - 25.f);
    updateThumbPosition();
}

void Slider::setSize(const sf::Vector2f& size)
{
    m_track.setSize(size);
    if (m_orientation == Orientation::Horizontal)
        m_thumb.setSize({ 10.f, size.y * 2 });
    else
        m_thumb.setSize({ size.x * 2, 10.f });

    updateThumbPosition();
}

void Slider::setValue(float value)
{
    value = std::clamp(value, m_min, m_max);
    float roundedValue = std::round((value - m_min) / m_step) * m_step + m_min;
    if (m_value != roundedValue) {
        m_value = roundedValue;
        updateThumbPosition();
        if (m_onValueChanged) {
            m_onValueChanged(m_value);
        }
    }
}

float Slider::getValue() const
{
    return m_value;
}

void Slider::setLabel(const std::wstring& text)
{
    m_label.setString(text);
}

void Slider::setOnValueChanged(std::function<void(float)> callback)
{
    m_onValueChanged = std::move(callback);
}

void Slider::setOnEventChanged(std::function<void()> callback)
{
    m_onEventChanged = std::move(callback);
}

void Slider::handleEvent(const sf::Event& event)
{
    if (!m_enabled || !m_visible) return;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (isThumbHovered(mousePos)) {
            m_dragging = true;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (m_dragging) {
            m_dragging = false;
            if (m_onEventChanged) {
                m_onEventChanged(); // 只在鼠标释放时调用事件回调
            }
        }
    }
    else if (event.type == sf::Event::MouseMoved && m_dragging) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        float pos;
        if (m_orientation == Orientation::Horizontal) {
            pos = mousePos.x - m_track.getPosition().x;
        }
        else {
            pos = mousePos.y - m_track.getPosition().y;
        }
        updateValueFromPosition(pos); // 拖动时实时更新数值，触发数值回调
    }
}

void Slider::update(float)
{
    // 不需要额外逻辑
}

void Slider::render(sf::RenderTarget& target)
{
    if (!m_visible) return;

    target.draw(m_track);
    target.draw(m_thumb);
    target.draw(m_label);
}

void Slider::updateThumbPosition()
{
    float ratio = (m_value - m_min) / (m_max - m_min);
    sf::Vector2f pos = m_track.getPosition();
    sf::Vector2f size = m_track.getSize();

    if (m_orientation == Orientation::Horizontal) {
        m_thumb.setPosition(pos.x + ratio * size.x - m_thumb.getSize().x / 2.f,
            pos.y - (m_thumb.getSize().y - size.y) / 2.f);
    }
    else {
        m_thumb.setPosition(pos.x - (m_thumb.getSize().x - size.x) / 2.f,
            pos.y + ratio * size.y - m_thumb.getSize().y / 2.f);
    }
}

void Slider::updateValueFromPosition(float pos)
{
    float totalLength = (m_orientation == Orientation::Horizontal) ? m_track.getSize().x : m_track.getSize().y;
    float ratio = std::clamp(pos / totalLength, 0.f, 1.f);
    float newValue = m_min + ratio * (m_max - m_min);
    setValue(newValue); // 触发实时数值回调
}

bool Slider::isThumbHovered(const sf::Vector2f& mousePos)
{
    return m_thumb.getGlobalBounds().contains(mousePos);
}
