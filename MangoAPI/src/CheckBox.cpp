#include "CheckBox.h"

CheckBox::CheckBox(const std::wstring& label, sf::Font& font) {
    m_box.setSize({ 20.f, 20.f });
    m_box.setFillColor(sf::Color::Transparent);
    m_box.setOutlineColor(sf::Color::White);
    m_box.setOutlineThickness(2.f);

    m_label.setFont(font);
    m_label.setString(label);
    m_label.setCharacterSize(18);
    m_label.setFillColor(sf::Color::White);
}

void CheckBox::setPosition(const sf::Vector2f& position) {
    m_box.setPosition(position);
    m_label.setPosition(position.x + 30.f, position.y - 2.f);
}

void CheckBox::setChecked(bool checked) {
    m_checked = checked;
}

bool CheckBox::isChecked() const {
    return m_checked;
}

void CheckBox::setCallback(std::function<void(bool)> callback) {
    m_callback = std::move(callback);
}

void CheckBox::handleEvent(const sf::Event& event) {
    if (!m_enabled || !m_visible) return;

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        auto mousePos = sf::Vector2f(static_cast<float>(event.mouseButton.x), (static_cast<float>(event.mouseButton.y)));
        if (m_box.getGlobalBounds().contains(mousePos)) {
            m_checked = !m_checked;
            if (m_callback)
                m_callback(m_checked);
        }
    }
}

void CheckBox::update(float) {}

void CheckBox::render(sf::RenderTarget& target) {
    if (!m_visible) return;

    target.draw(m_box);

    if (m_checked) {
        sf::RectangleShape mark({ 12.f, 12.f });
        mark.setFillColor(sf::Color::White);
        mark.setPosition(m_box.getPosition() + sf::Vector2f(4.f, 4.f));
        target.draw(mark);
    }

    target.draw(m_label);
}
