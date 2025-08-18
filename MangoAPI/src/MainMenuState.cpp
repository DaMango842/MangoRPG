#include "MainMenuState.h"
//#include "StateManager.h"
#include "ResourceLoader.h"
#include "GameState.h"
#include "SaveMenuState.h"
#include "SettingsMenuState.h"
#include "PlayerInfoState.h"

#ifdef DEV_MODE
#include "TestState.h"
#include "Player.h"
#endif // DEV_MODE


#include "Translator.h"

MainMenuState::MainMenuState(SafeRef<StateManager> stateManager)
    : m_stateManager(stateManager),
    m_font(SafeRef<sf::Font>(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf")))
{

}

void MainMenuState::setupUI() {
    m_title.setFont(*m_font);
    m_title.setString(TR("GAME_TITLE"));
    m_title.setCharacterSize(48);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(100.f, 50.f);

#ifdef DEV_MODE
    addButton(TR("TEST"), 150.f, [this]() {
        auto player = MangoPtr<Player>(Player::createDefaultData());
        m_stateManager->pushState(std::make_unique<TestState>(m_stateManager, player));
        });
#else
    addButton(TR("MAIN_MENU.NEW_GAME"), 150.f, [this]() {
        m_stateManager->pushState(std::make_unique<PlayerInfoState>(m_stateManager));
        });
#endif

    addButton(TR("MAIN_MENU.LOAD"), 250.f, [this]() {
        m_stateManager->pushState(std::make_unique<SaveMenuState>(m_stateManager, nullptr, SaveSlotType::Load));
        });
  

    addButton(TR("MAIN_MENU.SETTINGS"), 350.f, [this]() {
        m_stateManager->pushState(std::make_unique<SettingsMenuState>(m_stateManager));
        });

    addButton(TR("MAIN_MENU.EXIT_GAME"), 450.f, []() {
        std::exit(0);
        });
}

void MainMenuState::refreshUI()
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

void MainMenuState::addButton(const sf::String& text, float y, std::function<void()> callback)
{
    auto button = std::make_unique<Button>(text, *m_font);
    button->setPosition({ 100.f, y });
    button->setCallback(callback);
    m_components.push_back(std::move(button));
}

void MainMenuState::handleEvent(const sf::Event& event) {
    for (auto& comp : m_components) {
        comp->handleEvent(event);
    }
}

void MainMenuState::update(float deltaTime) {
    for (auto& comp : m_components) {
        comp->update(deltaTime);
    }
}

void MainMenuState::render(sf::RenderTarget& target) {
    target.draw(m_title);
    for (auto& comp : m_components) {
        comp->render(target);
    }
}

void MainMenuState::onEnter()
{
    //if (m_components.empty()) m_components.clear();
    setupUI();

    SafePtr<MainMenuState> self(this);

    SafeCallback<> cb(self, &MainMenuState::refreshUI);
    LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb();
        });
}

void MainMenuState::onExit()
{

}
