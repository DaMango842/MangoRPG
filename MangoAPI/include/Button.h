#pragma once

#include "BaseComponent.h"
#include <SFML/Graphics.hpp>
#include <functional>

/**
 * @brief 可点击的 UI 按钮组件，支持鼠标操作和键盘导航。
 *
 * Button 提供了基础的可视样式、点击响应、禁用控制，以及方向键导航支持（焦点切换）。
 */
class Button : public BaseComponent
{
public:
    /**
     * @brief 构造一个按钮。
     * @param text 显示在按钮上的文本。
     * @param font 使用的字体。
     * @param characterSize 字体大小（默认 24）。
     */
    Button(const sf::String& text, const sf::Font& font, unsigned int characterSize = 24);

    /**
     * @brief 处理事件（主要处理鼠标移动和点击）。
     * @param event SFML 事件。
     */
    void handleEvent(const sf::Event& event) override;

    /**
     * @brief 每帧更新按钮状态。
     * @param deltaTime 与时间相关的更新参数（当前未使用）。
     */
    void update(float deltaTime) override;

    /**
     * @brief 渲染按钮内容（形状 + 文字）。
     * @param target 目标渲染目标。
     */
    void render(sf::RenderTarget& target) override;

    /**
     * @brief 设置按钮的位置。
     * @param position 左上角位置。
     */
    void setPosition(const sf::Vector2f& position);

    /**
     * @brief 设置按钮的尺寸。
     * @param size 宽度和高度。
     */
    void setSize(const sf::Vector2f& size);

    /**
     * @brief 设置按钮按下时触发的回调函数。
     * @param callback 触发时调用的函数。
     */
    void setCallback(std::function<void()> callback);

    /**
     * @brief 启用或禁用按钮。
     * @param enabled 若为 false，按钮将灰显并失去响应。
     */
    void setEnabled(bool enabled);

    /**
     * @brief 查询按钮是否启用。
     * @return 是否可点击。
     */
    bool isEnabled() const;

    /**
     * @brief 设置按钮是否处于键盘聚焦状态。
     *
     * 聚焦状态用于方向键切换按钮时的视觉反馈（比如黄色边框）。
     * @param focused 是否聚焦。
     */
    void setFocused(bool focused);

    /**
     * @brief 查询按钮是否处于聚焦状态。
     * @return 是否聚焦。
     */
    bool isFocused() const;

    /**
     * @brief 模拟点击（用于方向键选中按钮后按下空格/回车）。
     */
    void simulateClick();

    /**
     * @brief 获取按钮的局部边界（不考虑变换）
     * @return 局部边界矩形
     */
    sf::FloatRect getLocalBounds() const;

    /**
     * @brief 获取按钮的全局边界（考虑变换）
     * @return 全局边界矩形
     */
    sf::FloatRect getGlobalBounds() const;

private:
    sf::RectangleShape m_shape; ///< 背景形状
    sf::Text m_text;            ///< 显示文本

    sf::Color m_normalColor = sf::Color(100, 100, 255);  ///< 默认颜色
    sf::Color m_hoverColor = sf::Color(120, 120, 255);  ///< 鼠标悬停颜色
    sf::Color m_pressedColor = sf::Color(80, 80, 200);    ///< 鼠标按下颜色
    sf::Color m_disabledColor = sf::Color(150, 150, 150);  ///< 禁用颜色
    sf::Color m_textColor = sf::Color::White;          ///< 字体颜色
    sf::Color m_focusOutlineColor = sf::Color::Yellow;     ///< 聚焦边框颜色

    std::function<void()> m_callback; ///< 点击时的回调函数

    bool m_isHovered = false; ///< 鼠标是否悬停
    bool m_isPressed = false; ///< 鼠标是否按下
    bool m_isEnabled = true;  ///< 是否启用
    bool m_isFocused = false; ///< 是否聚焦（键盘选中）
};
