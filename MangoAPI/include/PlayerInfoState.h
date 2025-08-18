#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "InputBox.h"
#include "Button.h"
#include "Player.h"
#include "SafeTypes.hpp"

#include <SFML/Graphics.hpp>
#include <cwctype>

#include <MangoPtr.hpp>

class PlayerInfoState : public BaseState {
public:
    PlayerInfoState(SafeRef<StateManager> stateManager);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

    bool isDone() const;
    const Player& getPlayer() const { return *m_player; }

    MangoPtr<Player> takePlayer();

private:
    void setupUI();
    void refreshUI();

private:
    SafeRef<StateManager> m_stateManager;
    MangoPtr<Player> m_player;

    sf::Font m_font;
    InputBox m_nameInput;
    InputBox m_goldInput;
    Button m_button;
    sf::Text m_title;

    bool m_done = false;
};
