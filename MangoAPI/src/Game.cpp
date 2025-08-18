#include "Game.h"
#include "MainMenuState.h"
#include "ResourceLoader.h"

#include "SafeTypes.hpp"

Game::Game()
    : m_fps(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{
    Window::getInstance().create(sf::VideoMode(1280, 800), TR("WINDOW_TITLE"));
    Window::getInstance().getWindow().setFramerateLimit(60);
    m_stateManager.changeState(std::make_unique<MainMenuState>(SafeRef<StateManager>(m_stateManager)));

    m_closeCallbackID = m_eventManager.subscribe(EventType::Closed, [](const sf::Event& event) -> bool {
        Window::getInstance().close();
        return true;
        });
}

void Game::run()
{
    while (Window::getInstance().isOpen())
    {
        float deltaTime = m_clock.restart().asSeconds();
        processEvents();

        // ✅ 应用待处理窗口变更（如全屏切换）
        Window::getInstance().applyPendingChanges();

        update(deltaTime);
        m_stateManager.applyPendingChange();
        render();
    }
}


void Game::processEvents()
{
    sf::Event event;
    while (Window::getInstance().getWindow().pollEvent(event))
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::F3) {
                m_fps.toggleVisible();
            }
        }
        
        m_eventManager.dispatchSingle(event);
        m_stateManager.handleEvent(event);
    }
}

void Game::update(float deltaTime)
{
    m_fps.update(deltaTime);
    m_stateManager.update(deltaTime);
}

void Game::render()
{
    Window::getInstance().getWindow().clear();

    m_fps.render(Window::getInstance().getWindow());
    m_stateManager.render(Window::getInstance().getWindow());

    Window::getInstance().getWindow().display();
}
