#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "Player.h"
#include "TileMap.h"
#include "FPS.h"
#include "Camera.h"
#include "Encounter.h"

#include "SafeTypes.hpp" // 引入 SafeRef 和 SafeHandle
#include <SFML/Graphics.hpp>

#include "Logger.h"

#include <MangoPtr.hpp>

class GameState : public BaseState
{
public:
    GameState(SafeRef<StateManager> stateManager, MangoPtr<Player> player);

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void onEnter() override;
    void onExit() override;

    Camera getCamera() const { return m_camera; }

private:
    SafeRef<StateManager> m_stateManager;
    MangoPtr<Player> m_player;
    TileMap m_tileMap;
    Camera m_camera;
    Encounter m_encounter;

    ENABLE_LOG_INJECTION(GameState);
};
