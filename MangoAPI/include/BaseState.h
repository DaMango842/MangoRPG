#pragma once

#include <SFML/Graphics.hpp>

class BaseState {
public:
    virtual ~BaseState() = default;

    // 核心交互
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    // 生命周期（状态切换）
    virtual void onEnter() {}
    virtual void onExit() {}

    // 暂停/恢复（用于状态堆叠）
    virtual void onPause() {}   // 当该状态被下压（另一个状态 push 上来）时调用
    virtual void onResume() {}  // 当该状态被恢复（上面的状态被 pop）时调用

    // 是否允许下层状态更新 / 渲染
    virtual bool allowUnderStateUpdate() const { return false; }
    virtual bool allowUnderStateRender() const { return false; }
};
