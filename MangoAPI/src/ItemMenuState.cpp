#include "ItemMenuState.h"
#include "StateManager.h"
#include "ResourceLoader.h"

ItemMenuState::ItemMenuState(SafeRef<StateManager> stateManager)
    : m_stateManager(stateManager),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
{
    m_title.setFont(m_font);
    m_title.setString(TR("INVENTORY.TITLE"));
    m_title.setCharacterSize(32);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(50.f, 50.f);
}

void ItemMenuState::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        m_stateManager->safePopState();
    }
}

void ItemMenuState::update(float) {}

void ItemMenuState::render(sf::RenderTarget& target) {
    target.draw(m_title);
}
