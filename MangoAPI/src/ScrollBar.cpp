#include "ScrollBar.h"
#include "Window.h"

ScrollBar::ScrollBar()
    : ScrollBar({ 0.f, 0.f }, { 0.f, 0.f }, ScrollDirection::Vertical) {
}

ScrollBar::ScrollBar(sf::Vector2f position, sf::Vector2f size, ScrollDirection direction)
    : m_direction(direction),
    m_min(0.f),
    m_max(100.f),
    m_value(0.f),
    m_contentHeight(0.f),
    m_viewHeight(0.f),
    m_thumbSize(0.f),
    m_dragging(false)
{
    m_track.setPosition(position);
    m_track.setSize(size);
    m_track.setFillColor(sf::Color(220, 230, 240));  // 浅灰蓝色背景轨道

    m_thumb.setFillColor(sf::Color(70, 130, 180));   // 钢蓝色滑块

    updateRangeAndThumbSize();
}

void ScrollBar::setRange(float min, float max) {
    m_min = min;
    m_max = max;
    m_value = std::clamp(m_value, m_min, m_max);
    updateThumb();
}

void ScrollBar::setValue(float value) {
    m_value = std::clamp(value, m_min, m_max);
    updateThumb();
}

float ScrollBar::getValue() const {
    return m_value;
}

float ScrollBar::getOffset() const {
    return getValue();
}

void ScrollBar::setContentHeight(float contentHeight) {
    m_contentHeight = contentHeight;
    updateRangeAndThumbSize();
}

void ScrollBar::setViewHeight(float viewHeight) {
    m_viewHeight = viewHeight;
    updateRangeAndThumbSize();
}

void ScrollBar::setSize(const sf::Vector2f& size) {
    m_track.setSize(size);
    updateRangeAndThumbSize();
}

void ScrollBar::setPosition(const sf::Vector2f& pos) {
    m_track.setPosition(pos);
    updateThumb();
}

float ScrollBar::getContentHeight() const {
    return m_contentHeight;
}

float ScrollBar::getViewHeight() const {
    return m_viewHeight;
}

void ScrollBar::update(float dt) {
    // 可根据需求添加惯性等逻辑
}

void ScrollBar::handleEvent(const sf::Event& event) {
    if (!m_enabled) return;  // 这里 m_enabled 由 BaseComponent 提供

    if (event.type == sf::Event::MouseWheelScrolled) {
        sf::Vector2f mousePos(
            static_cast<float>(event.mouseWheelScroll.x),
            static_cast<float>(event.mouseWheelScroll.y)
        );

        if (m_track.getGlobalBounds().contains(mousePos)) {
            if (m_direction == ScrollDirection::Vertical && event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                setValue(m_value - event.mouseWheelScroll.delta * 20.f);
            }
            else if (m_direction == ScrollDirection::Horizontal && event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel) {
                setValue(m_value - event.mouseWheelScroll.delta * 20.f);
            }
        }
    }
    else if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(
            static_cast<float>(event.mouseButton.x),
            static_cast<float>(event.mouseButton.y)
        );
        if (m_thumb.getGlobalBounds().contains(mousePos)) {
            m_dragging = true;
            m_dragOffset = mousePos - m_thumb.getPosition();
            m_thumb.setFillColor(sf::Color(50, 110, 160)); // 按下时颜色变深
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        m_dragging = false;
        m_thumb.setFillColor(sf::Color(70, 130, 180)); // 松开恢复正常颜色
    }
    else if (event.type == sf::Event::MouseMoved) {
        if (m_dragging) {
            sf::Vector2f mousePos(
                static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y)
            );
            sf::Vector2f newPos = mousePos - m_dragOffset;
            moveThumbTo(newPos);
        }
    }
    else if (event.type == sf::Event::LostFocus) {
        m_dragging = false;
        m_thumb.setFillColor(sf::Color(70, 130, 180)); // 恢复颜色
    }
}

void ScrollBar::render(sf::RenderTarget& target) {
    if (!m_visible) return;
    target.draw(m_track);

    // 只在有内容时绘制滑块
    if (m_contentHeight > m_viewHeight) {
        target.draw(m_thumb);
    }
}

void ScrollBar::updateRangeAndThumbSize() {
    sf::Vector2f trackSize = m_track.getSize();

    // 根据方向选择正确的长度
    float trackLength = (m_direction == ScrollDirection::Vertical) ? trackSize.y : trackSize.x;
    float contentLength = m_contentHeight; // 实际表示内容长度
    float viewLength = m_viewHeight;       // 实际表示视口长度

    // 添加保护性检查，防止除零错误
    if (contentLength <= 0.f || viewLength <= 0.f || contentLength <= viewLength) {
        m_thumbSize = 0.f; // 不需要滚动时，滑块大小为0
        setRange(0.f, 0.f);
        m_value = 0.f;
    }
    else {
        m_thumbSize = (viewLength / contentLength) * trackLength;
        m_thumbSize = std::clamp(m_thumbSize, 10.f, trackLength);
        setRange(0.f, contentLength - viewLength);
    }

    updateThumb();
}

void ScrollBar::updateThumb() {
    sf::Vector2f trackPos = m_track.getPosition();
    sf::Vector2f trackSize = m_track.getSize();

    // 避免除零错误
    if (m_max - m_min <= 0.001f) {
        return;
    }

    float rangeSpan = m_max - m_min;
    float ratio = (m_value - m_min) / rangeSpan;

    if (m_direction == ScrollDirection::Vertical) {
        m_thumb.setSize({ trackSize.x, m_thumbSize });
        float y = trackPos.y + ratio * (trackSize.y - m_thumbSize);
        m_thumb.setPosition(trackPos.x, y);
    }
    else {
        m_thumb.setSize({ m_thumbSize, trackSize.y });
        float x = trackPos.x + ratio * (trackSize.x - m_thumbSize);
        m_thumb.setPosition(x, trackPos.y);
    }
}

void ScrollBar::moveThumbTo(sf::Vector2f pos) {
    sf::Vector2f trackPos = m_track.getPosition();
    sf::Vector2f trackSize = m_track.getSize();

    if (m_direction == ScrollDirection::Vertical) {
        // 计算滑块可以移动的垂直范围
        float minY = trackPos.y;
        float maxY = trackPos.y + trackSize.y - m_thumbSize;

        // 确保滑块在轨道范围内
        float y = std::clamp(pos.y, minY, maxY);

        // 计算比例 (0-1)
        float ratio = (y - minY) / (maxY - minY);

        // 更新值
        m_value = m_min + ratio * (m_max - m_min);
    }
    else {
        // 水平滚动类似处理
        float minX = trackPos.x;
        float maxX = trackPos.x + trackSize.x - m_thumbSize;
        float x = std::clamp(pos.x, minX, maxX);
        float ratio = (x - minX) / (maxX - minX);
        m_value = m_min + ratio * (m_max - m_min);
    }

    updateThumb();
}
