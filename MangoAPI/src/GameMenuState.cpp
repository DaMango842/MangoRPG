#include "GameMenuState.h"
#include "GameState.h"
#include "MainMenuState.h"
#include "ResourceLoader.h"
#include "ItemMenuState.h"
#include "EquipMenuState.h"
#include "StatusMenuState.h"
#include "SaveMenuState.h"
#include "SettingsMenuState.h"

#include "Translator.h"

GameMenuState::GameMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player)
    : m_stateManager(stateManager),
    m_player(player),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{

}

void GameMenuState::setupUI()
{
    float startY = 100.f;
    float offsetY = 50.f;
    int index = 0;

    addButton(TR("GAME_MENU.CONTINUE"), startY + offsetY * index++, [this]() {
        m_stateManager->safePopState();
        });

    addButton(TR("GAME_MENU.ITEMS"), startY + offsetY * index++, [this]() {
        m_stateManager->pushState(std::make_unique<ItemMenuState>(m_stateManager));
        });

    addButton(TR("GAME_MENU.EQUIP"), startY + offsetY * index++, [this]() {
        m_stateManager->pushState(std::make_unique<EquipMenuState>(m_stateManager));
        });

    addButton(TR("GAME_MENU.STATUS"), startY + offsetY * index++, [this]() {
        m_stateManager->pushState(std::make_unique<StatusMenuState>(m_stateManager, m_player));
        });

    addButton(TR("GAME_MENU.SAVE"), startY + offsetY * index++, [this]() {
        m_stateManager->pushState(std::make_unique<SaveMenuState>(m_stateManager, m_player));
        });

    addButton(TR("GAME_MENU.SETTINGS"), startY + offsetY * index++, [this]() {
        m_stateManager->pushState(std::make_unique<SettingsMenuState>(m_stateManager));
        });

    addButton(TR("GAME_MENU.EXIT_TO_MAIN_MENU"), startY + offsetY * index++, [this]() {
        m_stateManager->safeChangeState(std::make_unique<MainMenuState>(m_stateManager));
        });
}

void GameMenuState::refreshUI()
{
    for (auto& comp : m_components) {
        if (!comp)
            continue;
        comp->setEnabled(false);
        comp->setVisible(false);
    }
    m_components.clear();
    setupUI();
}

void GameMenuState::addButton(const sf::String& text, float y, std::function<void()> callback)
{
    auto button = std::make_unique<Button>(text, m_font);
    button->setPosition({ 100.f, y });
    button->setCallback(callback);
    m_components.push_back(std::move(button));
}

void GameMenuState::handleEvent(const sf::Event& event)
{
    for (auto& comp : m_components)
        comp->handleEvent(event);
}

void GameMenuState::update(float deltaTime)
{
    for (auto& comp : m_components)
        comp->update(deltaTime);
}

void GameMenuState::render(sf::RenderTarget& target)
{
    for (auto& comp : m_components)
        comp->render(target);
}

void GameMenuState::onEnter()
{
    setupUI();

    SafePtr<GameMenuState> self(this);

    SafeCallback<> cb(self, &GameMenuState::refreshUI);
    LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb();
        });
}

void GameMenuState::onExit()
{
    m_components.clear();
    LOG_INFO("GameMenuState exited and buttons cleared.");
}
