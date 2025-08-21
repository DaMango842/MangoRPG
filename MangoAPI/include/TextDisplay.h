#pragma once
#include <SFML/Graphics.hpp>
#include <MangoString.hpp>

class TextDisplay {
public:
    TextDisplay(const sf::Font& font, unsigned int charSize = 30);

    void setText(const String& text, float displaySpeed = 0.05f);
    void update(float deltaTime);
    void complete();
    void reset();

    bool isComplete() const { return m_complete; }
    const String& getDisplayedText() const { return m_displayedText; }
    void setPosition(const sf::Vector2f& position);
    void setPosition(float x, float y);
    void setColor(const sf::Color& color);
    void draw(sf::RenderTarget& target);

    unsigned int getCharacterSize() const { return m_text.getCharacterSize(); }
    sf::FloatRect getLocalBounds() const { return m_text.getLocalBounds(); }

private:
    sf::Text m_text;
    String m_fullText;
    String m_displayedText;
    float m_displaySpeed;
    float m_charTimer;
    bool m_complete;
};
