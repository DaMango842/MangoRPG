#pragma once

#include <SFML/Graphics.hpp>
#include "BaseComponent.h"
#include "ScrollDirection.h"
#include <algorithm>

class ScrollBar : public BaseComponent {
public:
    ScrollBar();
    ScrollBar(sf::Vector2f position, sf::Vector2f size, ScrollDirection direction = ScrollDirection::Vertical);

    void setRange(float min, float max);
    void setValue(float value);
    float getValue() const;
    float getOffset() const;

    void setContentHeight(float contentHeight);
    void setViewHeight(float viewHeight);
    void setSize(const sf::Vector2f& size);
    void setPosition(const sf::Vector2f& pos);

    float getContentHeight() const;
    float getViewHeight() const;
    
    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;

private:
    void updateThumb();
    void moveThumbTo(sf::Vector2f pos);
    void updateRangeAndThumbSize();

private:
    sf::RectangleShape m_track;
    sf::RectangleShape m_thumb;

    float m_min = 0.f;
    float m_max = 100.f;
    float m_value = 0.f;

    float m_contentHeight = 0.f;
    float m_viewHeight = 0.f;
    float m_thumbSize = 0.f;

    bool m_dragging = false;
    sf::Vector2f m_dragOffset;

    ScrollDirection m_direction = ScrollDirection::Vertical;
};
