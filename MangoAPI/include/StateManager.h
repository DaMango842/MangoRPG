#pragma once

#include <deque>
#include <memory>
#include <format>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "BaseState.h"
#include "Logger.h"
#include "Utils.h"

class StateManager 
{
public:
    StateManager() = default;

    void pushState(std::unique_ptr<BaseState> state);
    void popState();
    void changeState(std::unique_ptr<BaseState> state);
    void clearStates();

    void safePopState();
    void safeChangeState(std::unique_ptr<BaseState> state);
    void queueStateChange(std::unique_ptr<BaseState> state);

    void applyPendingChange();

    void handleEvent(const sf::Event & event);
    void update(float deltaTime);
    void render(sf::RenderWindow & window);

    [[nodiscard]] BaseState* getCurrentState();

private:
    enum class PendingActionType {
        None,
        Pop,
        Push,
        Change,
        Clear
    };

    struct PendingAction {
        PendingActionType type = PendingActionType::None;
        std::unique_ptr<BaseState> state = nullptr;
    };

    std::deque<std::unique_ptr<BaseState>> m_states;
    PendingAction m_pendingAction;
};
