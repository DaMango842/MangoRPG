#include "InputBox.h"
#include <SFML/Window/Clipboard.hpp>

InputBox::InputBox(const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size) {
    m_box.setSize(size);
    m_box.setPosition(position);
    m_box.setFillColor(sf::Color(50, 50, 50));
    m_box.setOutlineColor(sf::Color::White);
    m_box.setOutlineThickness(2.f);

    m_text.setFont(font);
    m_text.setCharacterSize(24);
    m_text.setPosition(position.x + 5.f, position.y + 5.f);
    m_text.setFillColor(sf::Color::White);

    m_placeholder.setFont(font);
    m_placeholder.setCharacterSize(24);
    m_placeholder.setPosition(position.x + 5.f, position.y + 5.f);
    m_placeholder.setFillColor(sf::Color(150, 150, 150));

    m_error.setFont(font);
    m_error.setCharacterSize(18);
    m_error.setFillColor(sf::Color::Red);
    m_error.setPosition(position.x, position.y + size.y + 5.f);

    m_cursor.setFillColor(sf::Color::White);
    m_cursor.setSize({ 2.f, 24.f });
}

void InputBox::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        auto bounds = m_box.getGlobalBounds();
        m_isActive = bounds.contains(event.mouseButton.x, event.mouseButton.y);
        m_box.setOutlineColor(m_isActive ? sf::Color::Yellow : sf::Color::White);
    }

    if (!m_isActive) return;

    if (event.type == sf::Event::TextEntered) {
        wchar_t unicode = static_cast<wchar_t>(event.text.unicode);
        if (unicode >= 32 && unicode != 127) {
            if (!m_charFilter || m_charFilter(unicode)) {
                m_inputText.insert(m_inputText.begin() + m_cursorIndex, unicode);
                ++m_cursorIndex;
                updateTextDisplay();
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Backspace:
            if (m_cursorIndex > 0) {
                m_inputText.erase(m_cursorIndex - 1, 1);
                --m_cursorIndex;
                updateTextDisplay();
            }
            break;
        case sf::Keyboard::Left:
            if (m_cursorIndex > 0) --m_cursorIndex;
            break;
        case sf::Keyboard::Right:
            if (m_cursorIndex < m_inputText.size()) ++m_cursorIndex;
            break;
        case sf::Keyboard::V:
            if (event.key.control) {
                std::wstring clip = sf::Clipboard::getString().toWideString();
                m_inputText.insert(m_cursorIndex, clip);
                m_cursorIndex += clip.size();
                updateTextDisplay();
            }
            break;
        default:
            break;
        }
    }
}

void InputBox::update(float deltaTime) {
    m_cursorTimer += deltaTime;
    if (m_cursorTimer >= 0.5f) {
        m_cursorVisible = !m_cursorVisible;
        m_cursorTimer = 0.f;
    }
    updateCursorPosition();
}

void InputBox::render(sf::RenderTarget& target) {
    target.draw(m_box);
    if (m_inputText.empty() && !m_isActive) {
        target.draw(m_placeholder);
    }
    else {
        target.draw(m_text);
        if (m_isActive && m_cursorVisible)
            target.draw(m_cursor);
    }
    if (!m_error.getString().isEmpty())
        target.draw(m_error);
}

void InputBox::updateTextDisplay() {
    m_text.setString(m_inputText);
    updateCursorPosition();
}

void InputBox::updateCursorPosition() {
    sf::Vector2f pos = m_text.findCharacterPos(m_cursorIndex);
    m_cursor.setPosition(pos);
}

void InputBox::setText(const std::wstring& text) {
    m_inputText = text;
    m_cursorIndex = static_cast<int>(text.size());
    updateTextDisplay();
}

void InputBox::setPlaceholder(const std::wstring& text) {
    m_placeholder.setString(text);
}

void InputBox::setActive(bool active) {
    m_isActive = active;
    m_box.setOutlineColor(m_isActive ? sf::Color::Yellow : sf::Color::White);
}

void InputBox::setCharFilter(std::function<bool(wchar_t)> filter) {
    m_charFilter = std::move(filter);
}

void InputBox::setErrorMessage(const std::wstring& msg) {
    m_error.setString(msg);
}
