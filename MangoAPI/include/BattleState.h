#pragma once

#include "BaseState.h"
#include "StateManager.h"
#include "BattleSystem.h"  // 包含 BattleSystem 头文件
#include "Player.h"
#include "Enemy.h"
#include "Camera.h"
#include "Button.h"
#include "Label.h"
#include "ProgressBar.h"
#include "ButtonGroup.h"
#include "Container.h"
#include "Translator.h"
#include "SafeCallback.hpp"
#include "MangoPtr.hpp"
#include "MangoVector.hpp"
#include <SFML/Graphics.hpp>
#include <optional>

class BattleState : public BaseState {
public:
    BattleState(SafeRef<StateManager> stateManager, MangoPtr<Player> player);
    ~BattleState() override;

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;
    void onEnter() override;
    void onExit() override;

    void setManualEnemyCount(size_t count);

private:
    // UI Setup
    void setupUI();
    void createActionButton(const sf::String& label,
        const sf::Vector2f& pos,
        std::function<void()> callback);
    void updateUIElements();

    // Action handlers
    void onAttackButtonPressed();
    void onSkillButtonPressed();
    void onItemButtonPressed();
    void onDefendButtonPressed();
    void onEscapeButtonPressed();

    // Enemy Management
    void initializeEnemies();
    void layoutEnemies();
    MangoVector<MangoPtr<Enemy>> genEnemyGroup(size_t count);

    // 新增：处理敌人选择
    void handleEnemySelection(const sf::Event& event);

    // 成员变量
    SafeRef<StateManager>          m_stateManager;
    BattleSystem                   m_battleSys;  // 使用 BattleSystem 实例
    sf::Font& m_font;

    // Character references
    MangoPtr<Player>               m_player;
    MangoVector<MangoPtr<Enemy>>   m_enemies;

    // UI Elements
    struct PlayerUI {
        MangoPtr<Label>       name;
        MangoPtr<Label>       level;
        MangoPtr<ProgressBar> hpBar;
        MangoPtr<ProgressBar> mpBar;
    } m_playerUI;

    struct EnemyUI {
        MangoVector<MangoPtr<Label>>       names;
        MangoVector<MangoPtr<ProgressBar>> hpBars;
    } m_enemyUI;

    MangoPtr<ButtonGroup>          m_buttonGroup;
    Container                      m_container;

    // Battle state
    bool                           m_shouldExit{ false };
    size_t                         m_selectedEnemyIndex{ 0 };
    std::optional<size_t>          m_manualEnemyCount{ std::nullopt };

    // 新增：战斗子状态
    enum class SubState {
        PlayerActionSelection,  // 玩家选择行动
        EnemySelection,         // 选择敌人目标
        EnemyTurn,              // 敌人行动
        BattleOver              // 战斗结束
    };

    SubState m_currentSubState = SubState::PlayerActionSelection;

    ENABLE_LOG_INJECTION(BattleState);
};
