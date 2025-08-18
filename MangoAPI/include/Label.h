// Label.h
#pragma once

#include "BaseComponent.h"

class Label : public BaseComponent
{
public:
    Label(const sf::String& text, const sf::Font& font, unsigned int characterSize = 24);

    void setText(const sf::String& text);
    void setPosition(const sf::Vector2f& position);
    void setColor(const sf::Color& color);
    void setCharacterSize(unsigned int size);

    const sf::Text& getText() const;

    void handleEvent(const sf::Event& event) override {}
    void update(float deltaTime) override {}
    void render(sf::RenderTarget& target) override;

private:
    sf::Text m_text;
};
