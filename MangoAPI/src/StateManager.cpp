#include "StateManager.h"

void StateManager::pushState(std::unique_ptr<BaseState> state) {
    if (state) {
        LOG_INFO(std::format("Push state: {}", typeid(*state).name()));
        state->onEnter();
        m_states.push_back(std::move(state));
    }
}

void StateManager::popState() {
    if (!m_states.empty()) {
        LOG_INFO(std::format("Pop state: {}", typeid(*m_states.back()).name()));
        m_states.back()->onExit();
        m_states.pop_back();
    }
}

void StateManager::changeState(std::unique_ptr<BaseState> state) {
    popState();
    pushState(std::move(state));
}

void StateManager::clearStates() {
    for (auto& state : m_states) {
        if (state) {
            LOG_INFO(std::format("Clear state: {}", typeid(*state).name()));
            state->onExit();
        }
    }
    m_states.clear();
}

void StateManager::safePopState() {
    m_pendingAction = { PendingActionType::Pop, nullptr };
}

void StateManager::safeChangeState(std::unique_ptr<BaseState> state) {
    m_pendingAction = { PendingActionType::Change, std::move(state) };
}

void StateManager::queueStateChange(std::unique_ptr<BaseState> state) {
    m_pendingAction = { PendingActionType::Push, std::move(state) };
}

void StateManager::applyPendingChange() {
    switch (m_pendingAction.type) {
    case PendingActionType::Pop:
        LOG_INFO("Applying: Pop");
        popState();
        break;
    case PendingActionType::Push:
        LOG_INFO("Applying: Push");
        pushState(std::move(m_pendingAction.state));
        break;
    case PendingActionType::Change:
        LOG_INFO("Applying: Change");
        changeState(std::move(m_pendingAction.state));
        break;
    case PendingActionType::Clear:
        LOG_INFO("Applying: Clear");
        clearStates();
        break;
    default:
        break;
    }

    m_pendingAction = {};
}

void StateManager::handleEvent(const sf::Event& event) {
    if (!m_states.empty()) {
        m_states.back()->handleEvent(event);
    }
}

void StateManager::update(float deltaTime) {
    if (!m_states.empty()) {
        m_states.back()->update(deltaTime);
    }
}

void StateManager::render(sf::RenderWindow& window) {
    if (!m_states.empty()) {
        m_states.back()->render(window);
    }
}

BaseState* StateManager::getCurrentState() {
    return m_states.empty() ? nullptr : m_states.back().get();
}
