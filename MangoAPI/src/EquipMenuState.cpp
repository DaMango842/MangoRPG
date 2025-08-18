#include "EquipMenuState.h"
#include "ResourceLoader.h"

EquipMenuState::EquipMenuState(StateManager& stateManager)
    : m_stateManager(stateManager), m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf")) 
{
   

    setupUI();
}

void EquipMenuState::setupUI() {
    m_title.setFont(m_font);
    m_title.setString(TR("EQUIPMENT.TITLE"));
    m_title.setCharacterSize(36);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(100.f, 25.f);

    auto backBtn = std::make_unique<Button>(TR("BACKBTN"), m_font);
    backBtn->setPosition({ 100.f, 150.f });
    backBtn->setCallback([this]() {
        m_stateManager.safePopState();
        });

    m_buttons.push_back(std::move(backBtn));
}

void EquipMenuState::handleEvent(const sf::Event& event) {
    for (auto& btn : m_buttons) {
        btn->handleEvent(event);
    }
}

void EquipMenuState::update(float deltaTime) {
    for (auto& btn : m_buttons) {
        btn->update(deltaTime);
    }
}

void EquipMenuState::render(sf::RenderTarget& target) {
    target.draw(m_title);
    for (auto& btn : m_buttons) {
        btn->render(target);
    }
}

void EquipMenuState::onEnter()
{
    setupUI();

    LanguageObserver::get().subscribe([&]() {
        setupUI();
        });
}

void EquipMenuState::onExit()
{
}
