#pragma once

#include "BaseState.h"
#include "Button.h"
#include "Label.h"
#include "StateManager.h"

#include "Player.h"
#include "SafeTypes.hpp"
#include "SafeCallback.hpp"

#include "Translator.h"

#include <vector>

#include <MangoPtr.hpp>

class StatusMenuState : public BaseState {
public:
    StatusMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

    const Player& getPlayer() const { return *m_player; }

private:
    void setupUI();
    void refreshUI();
private:
    SafeRef<StateManager> m_stateManager;
    sf::Font& m_font;
    sf::Text m_title;
    std::vector<sf::Text> m_texts;
    std::vector<std::unique_ptr<BaseComponent>> m_components;

    MangoPtr<Player> m_player;

    CallbackID m_langObserverId = 0;
};
