#pragma once

#include "BaseComponent.h"
#include <functional>

class CheckBox : public BaseComponent {
public:
    CheckBox(const std::wstring& label, sf::Font& font);
    void setPosition(const sf::Vector2f& position);
    void setChecked(bool checked);
    bool isChecked() const;
    void setCallback(std::function<void(bool)> callback);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    sf::RectangleShape m_box;
    sf::Text m_label;
    bool m_checked = false;
    std::function<void(bool)> m_callback;
};
