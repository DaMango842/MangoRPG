// BaseComponent.h
#pragma once
#include <SFML/Graphics.hpp>

class BaseComponent {
public:
    BaseComponent() = default;
    virtual ~BaseComponent() noexcept = default;

    // 核心接口
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void render(sf::RenderTarget& target) = 0;
    virtual void update(float deltaTime) = 0;

    // 父节点管理（已优化）
    void setParent(BaseComponent* parent) { m_parent = parent; }
    BaseComponent* getParent() const noexcept { return m_parent; }

    // 状态管理
    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    bool isEnabled() const noexcept { return m_enabled; }

    void setVisible(bool visible) noexcept { m_visible = visible; }
    bool isVisible() const noexcept { return m_visible; }

protected:
    BaseComponent* m_parent = nullptr;
    bool m_enabled = true;
    bool m_visible = true;
};
