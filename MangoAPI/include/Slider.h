#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include "BaseComponent.h"

class Slider : public BaseComponent {
public:
    enum class Orientation {
        Horizontal,
        Vertical
    };

    Slider(float min, float max, float step, sf::Font& font, Orientation orientation = Orientation::Horizontal);

    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void setValue(float value);
    float getValue() const;

    void setLabel(const std::wstring& text);
    void setOnValueChanged(std::function<void(float)> callback);
    void setOnEventChanged(std::function<void()> callback);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void updateThumbPosition();
    void updateValueFromPosition(float pos);
    bool isThumbHovered(const sf::Vector2f& mousePos);

private:
    sf::RectangleShape m_track;
    sf::RectangleShape m_thumb;
    sf::Text m_label;
    sf::Font& m_font;

    float m_min;
    float m_max;
    float m_step;
    float m_value;

    Orientation m_orientation;
    bool m_dragging = false;

    std::function<void(float)> m_onValueChanged; // 更新数值
    std::function<void()> m_onEventChanged; // 鼠标释放后事件处理
};
