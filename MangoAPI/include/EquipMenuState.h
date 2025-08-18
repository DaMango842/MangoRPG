#pragma once

#include "BaseState.h"
#include "Button.h"
#include "StateManager.h"
#include <vector>
#include "Translator.h"


class EquipMenuState : public BaseState {
public:
    EquipMenuState(StateManager& stateManager);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

private:
    StateManager& m_stateManager;
    sf::Font& m_font;
    sf::Text m_title;
    std::vector<std::unique_ptr<Button>> m_buttons;

    void setupUI();
};
