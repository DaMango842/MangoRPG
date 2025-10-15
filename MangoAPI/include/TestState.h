#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "Player.h"
#include "TileMap.h"
#include "FPS.h"
#include "Camera.h"
#include "Encounter.h"

#include "Container.h"

#include "SafeTypes.hpp" // 引入 SafeRef 和 SafeHandle
#include <SFML/Graphics.hpp>

#include "Logger.h"

//#include "DialogueSystem.h"

#include <MangoPtr.hpp>

class TestState : public BaseState
{
public:
    TestState(SafeRef<StateManager> stateManager, MangoPtr<Player> player);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

    Camera getCamera() const { return m_camera; }

private:
    void setupUI();
    void refreshUI();
private:
    SafeRef<StateManager> m_stateManager;
    MangoPtr<Player> m_player;
    TileMap m_tileMap;
    Camera m_camera;
    Encounter m_encounter;

    Container m_container;
};
