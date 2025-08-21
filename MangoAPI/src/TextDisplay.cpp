#include "TextDisplay.h"

TextDisplay::TextDisplay(const sf::Font& font, unsigned int charSize)
    : m_displaySpeed(0.05f), m_charTimer(0.f), m_complete(true) {
    m_text.setFont(font);
    m_text.setCharacterSize(charSize);
    m_text.setFillColor(sf::Color::White);
}

void TextDisplay::setText(const String& text, float displaySpeed) {
    m_fullText = text;
    m_displayedText.clear();
    m_displaySpeed = displaySpeed;
    m_charTimer = 0.f;
    m_complete = false;
    m_text.setString(m_displayedText);
}

void TextDisplay::update(float deltaTime) {
    if (m_complete) return;

    m_charTimer += deltaTime;
    if (m_charTimer >= m_displaySpeed) {
        m_charTimer = 0.f;
        if (m_displayedText.size() < m_fullText.size()) {
            m_displayedText += m_fullText[m_displayedText.size()];
            m_text.setString(m_displayedText);
        }
        else {
            m_complete = true;
        }
    }
}

void TextDisplay::complete() {
    if (!m_complete) {
        m_displayedText = m_fullText;
        m_text.setString(m_displayedText);
        m_complete = true;
    }
}

void TextDisplay::reset() {
    m_displayedText.clear();
    m_fullText.clear();
    m_text.setString(m_displayedText);
    m_complete = true;
}

void TextDisplay::setPosition(const sf::Vector2f& position) {
    m_text.setPosition(position);
}

void TextDisplay::setPosition(float x, float y) {
    setPosition({ x, y });
}

void TextDisplay::setColor(const sf::Color& color) {
    m_text.setFillColor(color);
}

void TextDisplay::draw(sf::RenderTarget& target) {
    target.draw(m_text);
}
