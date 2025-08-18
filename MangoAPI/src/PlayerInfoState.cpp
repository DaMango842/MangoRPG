#include "PlayerInfoState.h"
#include "ResourceLoader.h"
#include "GameState.h"

PlayerInfoState::PlayerInfoState(SafeRef<StateManager> stateManager)
    : m_stateManager(stateManager),
    m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf")),
    m_nameInput(m_font, { 150.f, 120.f }, { 300.f, 40.f }),
    m_goldInput(m_font, { 150.f, 200.f }, { 300.f, 40.f }),
    m_button(L"确定", m_font),
    m_player(make_mango_ptr<Player>())
{
    setupUI();
}

void PlayerInfoState::setupUI() {
    m_nameInput.setPlaceholder(L"输入玩家名");
    m_nameInput.setCharFilter([](wchar_t c) {
        return std::iswalpha(c) || std::iswdigit(c);
        });

    m_goldInput.setPlaceholder(L"输入初始金币");
    m_goldInput.setCharFilter([](wchar_t c) {
        return std::iswdigit(c);
        });

    m_button.setSize({ 150.f, 40.f });
    m_button.setPosition({ 225.f, 280.f });
    m_button.setCallback([this]() {
        try {
            auto name = m_nameInput.getText();
            auto goldStr = m_goldInput.getText();

            if (name.empty()) {
                m_nameInput.setErrorMessage(L"玩家名不能为空");
                return;
            }

            int gold = std::stoi(goldStr);
            m_player->setName(name);
            auto& attr = m_player->getAttributes();
            attr.set("LV", 1);
            attr.set("LVMAX", 10);
            attr.set("HP", 100);
            attr.set("HPMAX", 100);
            attr.set("MANA", 0);
            attr.set("MANAMAX", 0);
            attr.set("ATK", 4);
            attr.set("DEF", 2);
            attr.set("MATK", 0);
            attr.set("MDEF", 0);
            attr.set("MONEY", static_cast<float>(gold));
            attr.set("EXP", 0);
            attr.set("NEXTEXP", 10);

            m_done = true;
        }
        catch (...) {
            m_goldInput.setErrorMessage(L"请输入有效的数字");
        }

        if (m_done) {
            m_stateManager->queueStateChange(
                std::make_unique<GameState>(m_stateManager, m_player)
            );
        }
        });

    m_title.setFont(m_font);
    m_title.setCharacterSize(28);
    m_title.setString(L"请输入玩家信息");
    m_title.setPosition(170.f, 50.f);
}

void PlayerInfoState::refreshUI() {
    setupUI();
}

void PlayerInfoState::handleEvent(const sf::Event& event) {
    m_nameInput.handleEvent(event);
    m_goldInput.handleEvent(event);
    m_button.handleEvent(event);
}

void PlayerInfoState::update(float deltaTime) {
    m_nameInput.update(deltaTime);
    m_goldInput.update(deltaTime);
}

void PlayerInfoState::render(sf::RenderTarget& target) {
    target.clear(sf::Color(30, 30, 30));
    target.draw(m_title);
    m_nameInput.render(target);
    m_goldInput.render(target);
    m_button.render(target);
}

void PlayerInfoState::onEnter()
{
    setupUI();

    LanguageObserver::get().subscribe([&]() {
        setupUI();
        });
}

void PlayerInfoState::onExit()
{
}

bool PlayerInfoState::isDone() const {
    return m_done;
}

MangoPtr<Player> PlayerInfoState::takePlayer() {
    assert(m_player && "Player not initialized or already taken");
    return m_player;
}
