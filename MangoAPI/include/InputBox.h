#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class InputBox {
public:
    InputBox(const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size);

    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderTarget& target);

    void setActive(bool active);
    bool isActive() const { return m_isActive; }

    std::wstring getText() const { return m_inputText; }
    void setText(const std::wstring& text);

    void setPlaceholder(const std::wstring& text);
    void setCharFilter(std::function<bool(wchar_t)> filter);
    void setErrorMessage(const std::wstring& msg);

private:
    void updateTextDisplay();
    void updateCursorPosition();

    sf::RectangleShape m_box;
    sf::Text m_text;
    sf::Text m_placeholder;
    sf::Text m_error;

    sf::RectangleShape m_cursor;
    float m_cursorTimer = 0.f;
    bool m_cursorVisible = true;

    std::wstring m_inputText;
    size_t m_cursorIndex = 0;

    bool m_isActive = false;

    std::function<bool(wchar_t)> m_charFilter;
};
