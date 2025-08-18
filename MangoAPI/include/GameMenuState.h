#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "Button.h"
#include "Player.h"
#include "SafeTypes.hpp"
#include "Logger.h"
#include <vector>
#include <SFML/Graphics.hpp>

#include <MangoPtr.hpp>

class GameMenuState : public BaseState
{
public:
    GameMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

    const Player& getPlayer() const { return *m_player; }

private:
    void setupUI();
    void refreshUI();

    void addButton(const sf::String& text, float y, std::function<void()> callback);

private:
    SafeRef<StateManager> m_stateManager;
    MangoPtr<Player> m_player;
    sf::Font& m_font;
    //sf::Text m_title;
    std::vector<std::unique_ptr<BaseComponent>> m_components;

    ENABLE_LOG_INJECTION(GameMenuState);
};
