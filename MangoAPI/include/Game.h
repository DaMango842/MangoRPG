#pragma once

#include <SFML/Graphics.hpp>
#include "StateManager.h"
#include "EventManager.h"
#include "Window.h"
#include "FPS.h"

#include "Logger.h"
#include "Translator.h"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();

private:
    sf::Clock m_clock;
    StateManager m_stateManager;
    EventManager m_eventManager;
    FPS m_fps;

    EventManager::CallbackID m_closeCallbackID;
    //EventManager::CallbackID m_eventCallbackID;
};
