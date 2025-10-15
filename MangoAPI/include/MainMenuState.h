#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "Button.h"
#include "ButtonGroup.h"
#include "SafeTypes.hpp" // 引入你刚才写的封装
#include <SFML/Graphics.hpp>
#include <vector>
#include "Utils.h"
#include "Logger.h"

//class StateManager;

#define DEV_MODE

class MainMenuState : public BaseState
{
public:
    explicit MainMenuState(SafeRef<StateManager> stateManager);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

private:
    void setupUI();
    void refreshUI();

    void addButton(const sf::String& text, float y, std::function<void()> callback);

private:
    SafeRef<StateManager> m_stateManager;
    SafeRef<sf::Font> m_font;

    sf::Text m_title;

    ButtonGroup m_btnGroup;

    std::vector<std::unique_ptr<BaseComponent>> m_components;

    ENABLE_LOG_INJECTION(MainMenuState);
};
