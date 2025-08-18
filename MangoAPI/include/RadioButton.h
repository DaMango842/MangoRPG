#pragma once
#include "BaseComponent.h"
#include <functional>
#include "SafeTypes.hpp"

class RadioBox;

class RadioButton : public BaseComponent {
public:
    RadioButton(const sf::String& label, sf::Font& font, SafePtr<RadioBox> group);

    void setPosition(const sf::Vector2f& pos);
    void setSelected(bool sel);
    bool isSelected() const;
    void setCallback(std::function<void()> cb);
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

    sf::String getLabel() const { return m_label.getString(); }
    bool getSelected() const { return m_selected; }


private:
    sf::CircleShape m_circle;
    sf::Text m_label;
    bool m_selected = false;
    std::function<void()> m_callback;
    SafePtr<RadioBox> m_group;
};
