#include "RadioBox.h"

void RadioBox::addOption(const sf::String& label, sf::Font& font, std::function<void()> callback) {
    auto btn = std::make_unique<RadioButton>(label, font, SafePtr<RadioBox>(this));
    if (callback) btn->setCallback(std::move(callback));
    m_buttons.push_back(std::move(btn));
}

void RadioBox::setPosition(const sf::Vector2f& startPos, float offsetY) {
    m_position = startPos;  
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i]->setPosition({ startPos.x, startPos.y + offsetY * i });
    }
}

sf::Vector2f RadioBox::getPosition() const {
    return m_position;
}

void RadioBox::handleEvent(const sf::Event& event) {
    if (!m_enabled || !m_visible) return;
    for (auto& btn : m_buttons)
        btn->handleEvent(event);
}

void RadioBox::update(float deltaTime) {
    for (auto& btn : m_buttons)
        btn->update(deltaTime);
}

void RadioBox::render(sf::RenderTarget& target) {
    if (!m_visible) return;
    for (auto& btn : m_buttons)
        btn->render(target);
}

void RadioBox::selectOnly(SafePtr<RadioButton> selectedBtn) {
    for (auto& btn : m_buttons)
        btn->setSelected(btn.get() == selectedBtn.get());
}

void RadioBox::selectOnly(bool selected) {
    // 未使用，可视情况删掉或实现
}

void RadioBox::selectOption(size_t index) {
    if (index < m_buttons.size())
        selectOnly(SafePtr<RadioButton>(m_buttons[index].get()));
}

RadioButton* RadioBox::getOption(size_t index) {
    return m_buttons[index].get();
}

bool RadioBox::getOptionSelected() {
    for (auto& btn : m_buttons)
        if (btn && btn->isSelected())
            return true;
    return false;
}

std::optional<size_t> RadioBox::getSelectedIndex() const {
    for (size_t i = 0; i < m_buttons.size(); ++i)
        if (m_buttons[i] && m_buttons[i]->isSelected())
            return i;
    return std::nullopt;
}
