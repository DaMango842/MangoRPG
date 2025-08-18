#pragma once

#include "BaseComponent.h"
#include <SFML/Graphics.hpp>
#include <optional>

class ProgressBar : public BaseComponent {
public:
    ProgressBar(float min, float max, float current, const sf::Vector2f& size);

    void setValue(float value);                      // 动画更新数值
    void setInstantValue(float value);               // 立即更新数值
    void setRange(float min, float max);             // 设置最小/最大
    void setSize(const sf::Vector2f& size);          // 设置尺寸
    void setPosition(const sf::Vector2f& pos);       // 设置位置

    void setBarColors(                                // 可选设置颜色（不传则保留默认）
        std::optional<sf::Color> full = std::nullopt,
        std::optional<sf::Color> mid = std::nullopt,
        std::optional<sf::Color> low = std::nullopt
    );
    void setBackgroundColor(const sf::Color& color);
    void showText(bool enable);                      // 开关文字显示

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void updateVisual();
    sf::Color getInterpolatedColor(float ratio);

    float m_min = 0.f;
    float m_max = 100.f;
    float m_currentValue = 100.f;
    float m_displayedValue = 100.f;
    sf::Vector2f m_size;
    sf::Vector2f m_position;

    // 色彩设置
    sf::Color m_colorFull = sf::Color::Green;
    sf::Color m_colorMid = sf::Color::Yellow;
    sf::Color m_colorLow = sf::Color::Red;
    sf::Color m_backgroundColor = { 60, 60, 60 };

    // 渲染
    sf::RectangleShape m_bgRect;
    sf::RectangleShape m_fgRect;
    sf::Font& m_font;
    sf::Text m_text;

    bool m_showText = true;
    float m_lerpSpeed = 5.f;
};
