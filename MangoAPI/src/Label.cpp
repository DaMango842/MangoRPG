// Label.cpp
#include "Label.h"

Label::Label(const sf::String& text, const sf::Font& font, unsigned int characterSize)
{
    m_text.setFont(font);
    m_text.setString(text);
    m_text.setCharacterSize(characterSize);
    m_text.setFillColor(sf::Color::White);
}

void Label::setText(const sf::String& text)
{
    m_text.setString(text);
}

void Label::setPosition(const sf::Vector2f& position)
{
    m_text.setPosition(position);
}

void Label::setColor(const sf::Color& color)
{
    m_text.setFillColor(color);
}

void Label::setCharacterSize(unsigned int size)
{
    m_text.setCharacterSize(size);
}

const sf::Text& Label::getText() const
{
    return m_text;
}

void Label::render(sf::RenderTarget& target)
{
    target.draw(m_text);
}
