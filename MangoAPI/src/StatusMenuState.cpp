#include "StatusMenuState.h"
#include "ResourceLoader.h"

StatusMenuState::StatusMenuState(SafeRef<StateManager> stateManager, MangoPtr<Player> player)
    : m_stateManager(stateManager),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf")),
    m_player(player)
{
    
}

void StatusMenuState::setupUI() {
    m_title.setFont(m_font);
    m_title.setString(L"状态菜单");
    m_title.setCharacterSize(36);
    m_title.setFillColor(sf::Color::White);
    m_title.setPosition(100.f, 25.f);

    float y = 100.f;

    sf::Text nameText;
    nameText.setFont(m_font);
    nameText.setCharacterSize(24);
    nameText.setFillColor(sf::Color::White);
    nameText.setPosition(100.f, y);
    nameText.setString(m_player->getName());

    m_texts.push_back(nameText);
    y += 40.f;

    for (const auto& key : attributeDisplayOrder) {
        if (!m_player->getAttributes().has(key)) continue;
        if (key == "LVMAX" || key == "HPMAX" || key == "MANAMAX") continue;

        sf::Text text;
        text.setFont(m_font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition(100.f, y);
        text.setString(m_player->getAttributes().getDisplayString(key));

        m_texts.push_back(text);
        y += 30.f;
    }

    auto backBtn = std::make_unique<Button>(TR("BACKBTN"), m_font);
    backBtn->setPosition({ 100.f, y + 150.f });
    backBtn->setCallback([this]() {
        m_stateManager->safePopState();
        });

    m_components.push_back(std::move(backBtn));
}

void StatusMenuState::refreshUI()
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

void StatusMenuState::handleEvent(const sf::Event& event) {
    for (auto& comp : m_components) {
        comp->handleEvent(event);
    }
}

void StatusMenuState::update(float deltaTime) {
    for (auto& comp : m_components) {
        comp->update(deltaTime);
    }
}

void StatusMenuState::render(sf::RenderTarget& target) {
    target.draw(m_title);

    for (const auto& text : m_texts) {
        target.draw(text);
    }

    for (auto& comp : m_components) {
        comp->render(target);
    }
}

void StatusMenuState::onEnter()
{
    setupUI();

    SafePtr<StatusMenuState> self(this);

    SafeCallback<> cb(self, &StatusMenuState::refreshUI);
    m_langObserverId = LanguageObserver::get().subscribe([cb]() mutable {
        if (cb) cb(); // 关键加这一层保护，防止 use-after-free
        });
}

void StatusMenuState::onExit()
{
    LanguageObserver::get().unsubscribe(m_langObserverId);
}
