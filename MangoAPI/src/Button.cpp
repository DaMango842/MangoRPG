#include "Button.h"
#include "Window.h"

Button::Button(const sf::String& text, const sf::Font& font, unsigned int characterSize)
{
    m_shape.setSize({ 150.f, 50.f });
    m_shape.setFillColor(m_normalColor);
    m_shape.setOutlineThickness(0.f); // 默认无边框

    m_text.setFont(font);
    m_text.setString(text);
    m_text.setCharacterSize(characterSize);
    m_text.setFillColor(m_textColor);

    // 初步居中（稍后可再修）
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f);
    m_text.setPosition(m_shape.getSize() / 2.f);
}

void Button::setPosition(const sf::Vector2f& position)
{
    m_shape.setPosition(position);

    // 更新文本位置（居中）
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f);
    m_text.setPosition(position.x + m_shape.getSize().x / 2.0f,
        position.y + m_shape.getSize().y / 2.0f);
}

void Button::setSize(const sf::Vector2f& size)
{
    m_shape.setSize(size);

    // 更新文本位置（居中）
    sf::Vector2f position = m_shape.getPosition();
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f);
    m_text.setPosition(position.x + size.x / 2.0f,
        position.y + size.y / 2.0f);
}

void Button::setCallback(std::function<void()> callback)
{
    m_callback = std::move(callback);
}

void Button::setEnabled(bool enabled)
{
    m_isEnabled = enabled;

    if (!m_isEnabled)
    {
        m_shape.setFillColor(m_disabledColor);
    }
    else
    {
        m_shape.setFillColor(m_normalColor);
    }
}

bool Button::isEnabled() const
{
    return m_isEnabled;
}

void Button::handleEvent(const sf::Event& event)
{
    if (!m_isEnabled) return;

    if (event.type == sf::Event::MouseMoved)
    {
        auto& window = Window::getInstance().getWindow();
        sf::Vector2f worldPos = window.mapPixelToCoords({ event.mouseMove.x, event.mouseMove.y });
        m_isHovered = m_shape.getGlobalBounds().contains(worldPos);
    }
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        auto& window = Window::getInstance().getWindow();
        sf::Vector2f worldPos = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
        if (m_shape.getGlobalBounds().contains(worldPos))
        {
            m_isPressed = true;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        auto& window = Window::getInstance().getWindow();
        sf::Vector2f worldPos = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
        if (m_isPressed && m_shape.getGlobalBounds().contains(worldPos))
        {
            if (m_callback)
                m_callback();
        }
        m_isPressed = false;
    }
}

void Button::update(float /*deltaTime*/)
{
    if (!m_isEnabled)
        return;

    if (m_isPressed)
        m_shape.setFillColor(m_pressedColor);
    else if (m_isHovered)
        m_shape.setFillColor(m_hoverColor);
    else
        m_shape.setFillColor(m_normalColor);
}

void Button::render(sf::RenderTarget& target)
{
    target.draw(m_shape);
    target.draw(m_text);
}

void Button::simulateClick()
{
    if (m_isEnabled && m_callback)
        m_callback();
}

sf::FloatRect Button::getLocalBounds() const
{
    return m_shape.getLocalBounds();
}

sf::FloatRect Button::getGlobalBounds() const
{
    return m_shape.getGlobalBounds();
}

void Button::setFocused(bool focused)
{
    m_isFocused = focused;

    if (m_isFocused)
    {
        m_shape.setOutlineThickness(2.f);
        m_shape.setOutlineColor(m_focusOutlineColor);
    }
    else
    {
        m_shape.setOutlineThickness(0.f);
    }
}

bool Button::isFocused() const
{
    return m_isFocused;
}
