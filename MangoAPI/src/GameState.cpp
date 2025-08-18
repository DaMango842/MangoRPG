#include "GameState.h"
#include "ResourceLoader.h"
#include "GameMenuState.h"
#include "Utils.h"

#include <iostream>

GameState::GameState(SafeRef<StateManager> stateManager, MangoPtr<Player> player)
    : m_stateManager(stateManager),
    m_camera({ 800.f, 640.f }),
    m_player(player)
{

}

void GameState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            m_camera.resetCamera(DEFAULT_SIZE);
            m_stateManager->pushState(
                std::make_unique<GameMenuState>(m_stateManager, m_player)
            );
        }
    }
}

void GameState::update(float deltaTime)
{
    m_player->update(deltaTime);
    m_camera.update(deltaTime);
    //m_fps.update(deltaTime);
}

void GameState::render(sf::RenderTarget& target)
{
    sf::View origView = target.getView();
    sf::View adjustedView = applyLetterboxView(m_camera.getView(), target.getSize().x, target.getSize().y);
    target.setView(adjustedView);

    target.draw(m_tileMap);
    m_player->render(target);

    m_encounter.update(*m_player, m_tileMap);

    target.setView(origView);
    //m_fps.render(target);
}

void GameState::onEnter()
{
    try {
        m_tileMap.loadFromJSON("Assets/MapData/test_map.json", "Assets/Image/Tileset/testTileset.png");
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    m_tileMap.setSolidTiles({ 42 });
    m_tileMap.setLayerVisible("Collision", false);

    m_player->setTileMap(&m_tileMap);

    m_camera.setSize(DEFAULT_SIZE);

    m_camera.setTarget(&m_player->getPosition());
    m_camera.setBounds({ 0, 0, m_tileMap.getMapSize().x, m_tileMap.getMapSize().y });



    m_player->initAnimator();
}

void GameState::onExit()
{
    m_camera.setTarget(nullptr);
    m_camera.resetCamera(DEFAULT_SIZE);
}
