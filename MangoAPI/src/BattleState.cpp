#include "BattleState.h"
#include "ResourceLoader.h"
#include "Window.h"
#include "Factory.h"
#include <Random.hpp>
#include <format>

BattleState::BattleState(SafeRef<StateManager> stateManager, MangoPtr<Player> player)
    : m_stateManager(stateManager)
    , m_battleSys()  // 初始化 BattleSystem
    , m_font(ResourceLoader::getFont("Assets/Font/fusion-pixel-12px.ttf"))
    , m_player(std::move(player))
    , m_currentSubState(SubState::PlayerActionSelection) {
}

BattleState::~BattleState() = default;

void BattleState::onEnter() {
    if (m_enemies.empty()) {
        Factory::get().loadEnemies();
        initializeEnemies();
    }
    layoutEnemies();
    setupUI();

    SafeCallback<> cb{ SafePtr<BattleState>(this), &BattleState::updateUIElements };
    LanguageObserver::get().subscribe(std::move(cb));
}

void BattleState::onExit() {
    m_enemies.clear();
    m_container.clearComponents();
}

void BattleState::handleEvent(const sf::Event& event) {
    // 根据当前子状态分发事件
    switch (m_currentSubState) {
    case SubState::PlayerActionSelection:
        // 玩家行动选择：由按钮组处理
        m_container.handleEvent(event);
        break;

    case SubState::EnemySelection:
        // 敌人选择状态：特殊处理
        handleEnemySelection(event);
        break;

    case SubState::EnemyTurn:
    case SubState::BattleOver:
        // 这些状态不处理输入
        break;
    }
}

void BattleState::handleEnemySelection(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Left:
            do {
                if (m_selectedEnemyIndex == 0)
                    m_selectedEnemyIndex = m_enemies.size() - 1;
                else
                    --m_selectedEnemyIndex;
            } while (m_selectedEnemyIndex < m_enemies.size() &&
                !m_battleSys.isEnemyAlive(m_selectedEnemyIndex));
            break;

        case sf::Keyboard::Right:
            do {
                m_selectedEnemyIndex = (m_selectedEnemyIndex + 1) % m_enemies.size();
            } while (m_selectedEnemyIndex < m_enemies.size() &&
                !m_battleSys.isEnemyAlive(m_selectedEnemyIndex));
            break;

        case sf::Keyboard::Z:
        case sf::Keyboard::Enter:
            if (m_selectedEnemyIndex < m_enemies.size() &&
                m_battleSys.isEnemyAlive(m_selectedEnemyIndex)) {

                // 提交玩家攻击动作
                m_battleSys.setSelectedEnemyIndex(m_selectedEnemyIndex);
                m_battleSys.submitPlayerAction(PlayerAction::Attack);

                // 推进战斗系统
                m_battleSys.advance();

                // 进入敌人行动阶段
                m_currentSubState = SubState::EnemyTurn;
            }
            break;

        case sf::Keyboard::X:
        case sf::Keyboard::Escape:
            // 取消选择，返回行动选择
            m_currentSubState = SubState::PlayerActionSelection;
            break;

        default:
            break;
        }
    }
}

void BattleState::update(float deltaTime) {
    // 检查战斗是否结束
    if (m_battleSys.isBattleOver()) {
        m_currentSubState = SubState::BattleOver;

        if (m_battleSys.isVictory()) {
            LOG_INFO("战斗胜利！");
            m_stateManager->safePopState();
            return;
        }
        else if (m_battleSys.isDefeated()) {
            LOG_INFO("游戏结束!");
            // 处理游戏结束逻辑
            return;
        }
    }

    // 处理敌人行动阶段
    if (m_currentSubState == SubState::EnemyTurn) {
        // 推进战斗系统（执行敌人行动）
        m_battleSys.advance();

        // 检查是否回到玩家回合
        if (m_battleSys.getStage() == BattleStage::PlayerSelecting) {
            m_currentSubState = SubState::PlayerActionSelection;
        }
    }

    // 更新UI
    updateUIElements();
    m_container.update(deltaTime);
}

void BattleState::render(sf::RenderTarget& target) {
    // 1. 渲染所有存活的敌人精灵
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        const auto& enemy = m_enemies[i];
        if (enemy && enemy->getAttributes().get<float>("HP") > 0.f) {
            target.draw(enemy->getSprite());
        }
    }

    // 2. 在敌人选择状态下渲染选择高亮效果
    if (m_currentSubState == SubState::EnemySelection) {
        for (size_t i = 0; i < m_enemies.size(); ++i) {
            const auto& enemy = m_enemies[i];
            if (enemy && enemy->getAttributes().get<float>("HP") > 0.f && i == m_selectedEnemyIndex) {
                // 创建高亮效果
                sf::Sprite hl = enemy->getSprite();
                hl.setColor(sf::Color(100, 100, 255, 128)); // 蓝色半透明高亮
                target.draw(hl);
            }
        }
    }

    // 3. 渲染UI容器（按钮、血条等）
    m_container.render(target);

    // 4. 在特定状态下渲染额外UI元素
    if (m_currentSubState == SubState::BattleOver) {
        // 渲染战斗结果信息
        sf::Text resultText;
        resultText.setFont(m_font);
        resultText.setCharacterSize(48);
        resultText.setFillColor(sf::Color::White);

        if (m_battleSys.isVictory()) {
            resultText.setString("Victory!");
        }
        else {
            resultText.setString("Defeat...");
        }

        // 居中显示
        sf::FloatRect bounds = resultText.getLocalBounds();
        resultText.setPosition(
            (Window::getInstance().getWindow().getSize().x - bounds.width) / 2.f,
            (Window::getInstance().getWindow().getSize().y - bounds.height) / 2.f
        );

        target.draw(resultText);
    }
}

void BattleState::setManualEnemyCount(size_t count) {
    m_manualEnemyCount = count;
}

void BattleState::setupUI() {
    m_container.clearComponents();

    // Player UI
    const sf::Vector2f basePos{ 250.f, 540.f };
    constexpr float rowGap = 32.f;
    constexpr float colGap = 140.f;

    m_playerUI.name = make_mango_ptr<Label>(m_player->getName(), m_font);
    m_playerUI.level = make_mango_ptr<Label>(std::format("Lv {}", m_player->getAttributes().get<int>("LV")), m_font);
    m_playerUI.hpBar = make_mango_ptr<ProgressBar>(
        0.f,
        m_player->getAttributes().get<float>("HPMAX"),
        m_player->getAttributes().get<float>("HP"),
        sf::Vector2f{ 240.f, 24.f });
    m_playerUI.mpBar = make_mango_ptr<ProgressBar>(
        0.f,
        m_player->getAttributes().get<float>("MANAMAX"),
        m_player->getAttributes().get<float>("MANA"),
        sf::Vector2f{ 240.f, 24.f });

    m_playerUI.name->setPosition(basePos);
    m_playerUI.level->setPosition(basePos + sf::Vector2f{ colGap, 0 });
    m_playerUI.hpBar->setPosition(basePos + sf::Vector2f{ colGap, rowGap });
    m_playerUI.mpBar->setPosition(basePos + sf::Vector2f{ colGap, rowGap * 2 });

    m_playerUI.hpBar->showText(true);
    m_playerUI.mpBar->showText(true);

    m_container.bindComponent(m_playerUI.name);
    m_container.bindComponent(m_playerUI.level);
    m_container.bindComponent(m_playerUI.hpBar);
    m_container.bindComponent(m_playerUI.mpBar);

    // Enemy UI
    m_enemyUI.names.clear();
    m_enemyUI.hpBars.clear();

    if (!m_enemies.empty()) {
        const auto& window = Window::getInstance().getWindow();
        const sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
        const size_t enemyCount = std::min(m_enemies.size(), size_t(4));

        constexpr float minButtonWidth = 160.f;
        const float maxTotalWidth = windowSize.x - 100.f;
        constexpr float defaultSpacing = 40.f;

        float buttonWidth = 200.f;
        float spacing = defaultSpacing;

        // Adjust layout if needed
        float totalWidth = enemyCount * buttonWidth + (enemyCount - 1) * spacing;
        if (totalWidth > maxTotalWidth) {
            buttonWidth = (maxTotalWidth - (enemyCount - 1) * spacing) / enemyCount;
            buttonWidth = std::max(buttonWidth, minButtonWidth);
            spacing = (maxTotalWidth - enemyCount * buttonWidth) / ((enemyCount > 1) ? enemyCount - 1 : 1);
        }

        float startX = (windowSize.x - (enemyCount * buttonWidth + (enemyCount - 1) * spacing)) / 2.f;
        constexpr float baseY = 80.f;

        for (size_t i = 0; i < enemyCount; ++i) {
            const sf::Vector2f pos{ startX + i * (buttonWidth + spacing), baseY };

            auto nameLabel = make_mango_ptr<Label>(m_enemies[i]->getName(), m_font);
            nameLabel->setPosition(pos);
            m_enemyUI.names.push_back(nameLabel);
            m_container.bindComponent(nameLabel);

            auto hpBar = make_mango_ptr<ProgressBar>(
                0.f,
                m_enemies[i]->getAttributes().get<float>("HPMAX"),
                m_enemies[i]->getAttributes().get<float>("HP"),
                sf::Vector2f{ buttonWidth, 20.f });
            hpBar->setPosition(pos + sf::Vector2f{ 0.f, 32.f });
            hpBar->showText(true);
            m_enemyUI.hpBars.push_back(hpBar);
            m_container.bindComponent(hpBar);
        }
    }

    // Action buttons
    m_buttonGroup = make_mango_ptr<ButtonGroup>();
    m_container.bindComponent(m_buttonGroup);

    const sf::Vector2f actionBasePos{ 50.f, 540.f };
    static constexpr std::array<const wchar_t*, 5> buttonTexts{
        L"攻 击", L"技 能", L"道 具", L"防 御", L"逃 跑"
    };

    for (int i = 0; i < 5; ++i) {
        createActionButton(
            sf::String(buttonTexts[i]),
            actionBasePos + sf::Vector2f{ 0.f, 48.f * i },
            [this, i] {
                switch (i) {
                case 0: onAttackButtonPressed(); break;
                case 3: m_battleSys.setDefending(true); break;
                case 4: if (Random::chance(0.5f)) m_shouldExit = true; break;
                default: break;
                }
            });
    }
}

void BattleState::updateUIElements() {
    // Update player UI
    m_playerUI.name->setText(m_player->getName());
    m_playerUI.level->setText(std::format("Lv {}", m_player->getAttributes().get<int>("LV")));
    m_playerUI.hpBar->setValue(m_player->getAttributes().get<float>("HP"));
    m_playerUI.mpBar->setValue(m_player->getAttributes().get<float>("MANA"));

    // Update enemy UI
    for (size_t i = 0; i < m_enemyUI.hpBars.size() && i < m_enemies.size(); ++i) {
        auto& enemy = m_enemies[i];
        if (!enemy || enemy->getAttributes().get<float>("HP") <= 0.f) {
            // 死亡敌人隐藏 UI
            m_enemyUI.names[i]->setVisible(false);
            m_enemyUI.hpBars[i]->setVisible(false);
            continue;
        }

        m_enemyUI.names[i]->setVisible(true);
        m_enemyUI.hpBars[i]->setVisible(true);

        m_enemyUI.names[i]->setText(enemy->getName());
        m_enemyUI.hpBars[i]->setValue(enemy->getAttributes().get<float>("HP"));
    }
}

void BattleState::onAttackButtonPressed() {
    // 检查是否有存活的敌人
    bool hasAliveEnemy = false;
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        if (m_battleSys.isEnemyAlive(i)) {
            hasAliveEnemy = true;
            m_selectedEnemyIndex = i;
            break;
        }
    }

    if (hasAliveEnemy) {
        m_currentSubState = SubState::EnemySelection;
    }
}

void BattleState::onDefendButtonPressed() {
    // 提交防御动作
    m_battleSys.setDefending(true);
    m_battleSys.submitPlayerAction(PlayerAction::Defend);
    m_battleSys.advance();

    // 进入敌人行动阶段
    m_currentSubState = SubState::EnemyTurn;
}

void BattleState::onEscapeButtonPressed() {
    // 提交逃跑动作
    m_battleSys.submitPlayerAction(PlayerAction::Flee);
    m_battleSys.advance();

    if (m_battleSys.isBattleOver() && !m_battleSys.isDefeated()) {
        // 逃跑成功
        m_stateManager->safePopState();
    }
    else {
        // 逃跑失败，进入敌人行动阶段
        m_currentSubState = SubState::EnemyTurn;
    }
}

void BattleState::initializeEnemies() {
    const size_t count = m_manualEnemyCount.value_or(static_cast<size_t>(Random::randint(1, 4)));
    m_enemies = genEnemyGroup(count);
    m_battleSys.initBattle(m_player, m_enemies);
}

void BattleState::layoutEnemies() {
    // Get window size - now properly handling runtime values
    const auto& window = Window::getInstance().getWindow();
    const sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());

    const size_t enemyCount = std::min(m_enemies.size(), size_t(4));
    if (enemyCount == 0) return;

    // Layout parameters
    constexpr float minEnemyWidth = 160.f;
    const float maxTotalWidth = windowSize.x - 100.f;
    constexpr float defaultSpacing = 40.f;

    // Calculate dynamic layout
    float enemyWidth = 200.f;
    float spacing = defaultSpacing;

    // Adjust for window size
    float totalWidth = enemyCount * enemyWidth + (enemyCount - 1) * spacing;
    if (totalWidth > maxTotalWidth) {
        enemyWidth = (maxTotalWidth - (enemyCount - 1) * spacing) / enemyCount;
        enemyWidth = std::max(enemyWidth, minEnemyWidth);
        spacing = (maxTotalWidth - enemyCount * enemyWidth) / ((enemyCount > 1) ? enemyCount - 1 : 1);
    }

    // Calculate starting position
    const float startX = (windowSize.x - totalWidth) / 2.f;
    constexpr float baseY = 450.f;

    // Position enemies
    for (size_t i = 0; i < enemyCount; ++i) {
        if (auto& enemy = m_enemies[i]) {
            enemy->setPosition({
                startX + i * (enemyWidth + spacing) + enemyWidth * 0.5f,
                baseY
                });
        }
    }
}

MangoVector<MangoPtr<Enemy>> BattleState::genEnemyGroup(size_t count) {
    MangoVector<MangoPtr<Enemy>> enemies;
    const auto& enemyDefs = Factory::get().getEnemies();

    if (enemyDefs.empty() || count == 0) return enemies;

    std::vector<std::string> enemyNames;
    enemyNames.reserve(enemyDefs.size());
    for (const auto& [name, _] : enemyDefs) {
        enemyNames.push_back(name);
    }

    enemies.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const auto& name = Random::choice(enemyNames);
        enemies.push_back(make_mango_ptr<Enemy>(*Factory::get().createEnemy(name)));
    }

    return enemies;
}

void BattleState::createActionButton(const sf::String& label,
    const sf::Vector2f& pos,
    std::function<void()> callback) {
    auto button = make_mango_ptr<Button>(label, m_font);
    button->setPosition(pos);
    button->setSize({ 120.f, 40.f });
    button->setCallback(std::move(callback));
    m_buttonGroup->addButton(button);
    m_container.bindComponent(button);
}
