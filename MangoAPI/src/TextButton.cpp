#include "TextButton.h"

TextButton::TextButton(const sf::String& text, const sf::Font& font, unsigned int characterSize)
{
    m_text.setFont(font);
    m_text.setString(text);
    m_text.setCharacterSize(characterSize);
    m_text.setFillColor(sf::Color::White);

    m_normalColor = sf::Color::White;
    m_hoverColor = sf::Color(200, 200, 255);
    m_pressedColor = sf::Color(150, 150, 255);
}

void TextButton::setPosition(const sf::Vector2f& position)
{
    m_text.setPosition(position);
}

void TextButton::setCallback(std::function<void()> callback)
{
    m_callback = callback;
}

void TextButton::setNormalColor(const sf::Color& color)
{
    m_normalColor = color;
}

void TextButton::setHoverColor(const sf::Color& color)
{
    m_hoverColor = color;
}

void TextButton::setPressedColor(const sf::Color& color)
{
    m_pressedColor = color;
}

void TextButton::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        auto bounds = m_text.getGlobalBounds();
        m_isHovered = bounds.contains(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
    }
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        auto bounds = m_text.getGlobalBounds();
        if (bounds.contains(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)))
        {
            m_isPressed = true;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        auto bounds = m_text.getGlobalBounds();
        if (m_isPressed && bounds.contains(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)))
        {
            if (m_callback)
                m_callback();
        }
        m_isPressed = false;
    }
}

void TextButton::update(float deltaTime)
{
    if (m_isPressed)
        m_text.setFillColor(m_pressedColor);
    else if (m_isHovered)
        m_text.setFillColor(m_hoverColor);
    else
        m_text.setFillColor(m_normalColor);
}

void TextButton::render(sf::RenderTarget& target)
{
    target.draw(m_text);
}
