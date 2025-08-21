#include "TestState.h"
#include "ResourceLoader.h"
#include "GameMenuState.h"
#include "Utils.h"

#include "BattleState.h"

#include <iostream>

TestState::TestState(SafeRef<StateManager> stateManager, MangoPtr<Player> player)
    : m_stateManager(stateManager),
    m_camera({ 800.f, 640.f }),
    m_player(std::move(player)),
	m_dialogSys(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{

}

void TestState::setupUI() {
    //m_container.clearComponents();

    //auto button = std::make_unique<Button>(L"测试战斗", ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"));
    //button->setSize({ 200.f, 75.f });
    //button->setPosition({ 600.f, 480.f });
    //button->setCallback([this]() {
    //    m_stateManager->pushState(std::make_unique<BattleState>(m_stateManager));
    //    });
    //m_container.bindComponent(std::move(button));
}

void TestState::refreshUI()
{
    setupUI();
}

void TestState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::F2)
        {
            m_camera.resetCamera(DEFAULT_SIZE);
            auto battleState = std::make_unique<BattleState>(m_stateManager, m_player);
            battleState->setManualEnemyCount(0x1);
            m_stateManager->pushState(std::move(battleState));
        }
    }

    m_container.handleEvent(event);
}

void TestState::update(float deltaTime)
{
    m_player->update(deltaTime);
    m_camera.update(deltaTime);
    m_container.update(deltaTime);
}

void TestState::render(sf::RenderTarget& target)
{
    sf::View origView = target.getView();
    sf::View adjustedView = applyLetterboxView(m_camera.getView(), target.getSize().x, target.getSize().y);
    target.setView(adjustedView);

    target.draw(m_tileMap);
    m_player->render(target);

    m_container.render(target);

    //m_encounter.update(*m_player, m_tileMap);

    target.setView(origView);
    //m_fps.render(target);
}

void TestState::onEnter()
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


    setupUI();

    SafePtr<TestState> self(this);

    SafeCallback<> cb(self, &TestState::refreshUI);
    LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb();
        });

}

void TestState::onExit()
{
    m_camera.setTarget(nullptr);
    m_camera.resetCamera(DEFAULT_SIZE);
}
