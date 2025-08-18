#pragma once

#include "BaseComponent.h"
#include <SFML/Graphics.hpp>
#include <functional>

class TextButton : public BaseComponent
{
public:
    TextButton(const sf::String& text, const sf::Font& font, unsigned int characterSize = 24);

    void setPosition(const sf::Vector2f& position);
    void setCallback(std::function<void()> callback);

    void setNormalColor(const sf::Color& color);
    void setHoverColor(const sf::Color& color);
    void setPressedColor(const sf::Color& color);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    sf::Text m_text;

    sf::Color m_normalColor;
    sf::Color m_hoverColor;
    sf::Color m_pressedColor;

    bool m_isHovered = false;
    bool m_isPressed = false;

    std::function<void()> m_callback;
};
