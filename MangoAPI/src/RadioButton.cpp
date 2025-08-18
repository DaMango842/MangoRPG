#include "RadioButton.h"
#include "RadioBox.h"

#include <iostream>

RadioButton::RadioButton(const sf::String& label, sf::Font& font, SafePtr<RadioBox> group)
    : m_group(group)
{
    m_circle.setRadius(10.f);
    m_circle.setFillColor(sf::Color::Transparent);
    m_circle.setOutlineColor(sf::Color::White);
    m_circle.setOutlineThickness(2.f);

    m_label.setFont(font);
    m_label.setString(label);
    m_label.setCharacterSize(18);
    m_label.setFillColor(sf::Color::White);
}

void RadioButton::setPosition(const sf::Vector2f& pos) {
    m_circle.setPosition(pos);
    m_label.setPosition(pos.x + 30.f, pos.y - 4.f);
}

void RadioButton::setSelected(bool sel) {
    m_selected = sel;
    //std::cout << "RadioButton setSelected: " << sel << std::endl;
}

bool RadioButton::isSelected() const { return m_selected; }
void RadioButton::setCallback(std::function<void()> cb) { m_callback = std::move(cb); }

void RadioButton::handleEvent(const sf::Event& event) {
    if (!m_enabled || !m_visible) return;
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        auto mousePos = sf::Vector2f((float)event.mouseButton.x, (float)event.mouseButton.y);
        if (m_circle.getGlobalBounds().contains(mousePos)) {
            //std::cout << "Clicked RadioButton this ptr: " << this << std::endl;
            if (!m_selected && m_group) {
                m_group->selectOnly(SafePtr<RadioButton>(this));
                if (m_callback) m_callback();
            }
        }
    }
}


void RadioButton::update(float) {}
void RadioButton::render(sf::RenderTarget& target) {
    if (!m_visible) return;
    target.draw(m_circle);
    if (m_selected) {
        sf::CircleShape dot(5.f);
        dot.setFillColor(sf::Color::White);
        dot.setPosition(m_circle.getPosition() + sf::Vector2f(5.f, 5.f));
        target.draw(dot);
    }
    target.draw(m_label);
}
