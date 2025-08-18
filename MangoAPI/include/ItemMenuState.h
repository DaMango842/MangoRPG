#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include <SFML/Graphics.hpp>
#include "Utils.h"
#include "Translator.h"

class ItemMenuState : public BaseState 
{
public:
    ItemMenuState(SafeRef<StateManager> stateManager);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    SafeRef<StateManager> m_stateManager;
    sf::Font& m_font;
    sf::Text m_title;
};
