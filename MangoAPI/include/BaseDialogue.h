// BaseDialogue.h
#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class BaseDialogue {
public:
    enum class State {
        Inactive,
        Active,
        Paused,
        Finished
    };

    virtual ~BaseDialogue() = default;

    // 游戏循环相关
    // 纯事件
    virtual void handleEvent(const sf::Event& event) {};
    // 用于获取坐标等一些东西的时候才会用的方法
    virtual void handleEvent(const sf::Event& event, const sf::RenderTarget& target) {};
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    // 生命周期控制
    virtual void startDialogue() = 0;
    virtual void endDialogue() = 0;
    virtual void pauseDialogue() {}
    virtual void resumeDialogue() {}

    // 状态查询
    virtual State getState() const = 0;
    virtual bool wantsInputFocus() const { return true; }

    // 设置对话结束回调
    void setOnDialogueEndedCallback(std::function<void()> callback) {
        m_onDialogueEnded = callback;
    }

protected:
    std::function<void()> m_onDialogueEnded;
};
