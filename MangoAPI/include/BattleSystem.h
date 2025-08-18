#pragma once

#include <vector>
#include "Player.h"
#include "Enemy.h"

#include "MangoPtr.hpp"
#include "MangoVector.hpp"
#include "SafeTypes.hpp"

enum class BattleStage {
    Init,
    PlayerSelecting,       // 玩家选择行为（攻击、防御、逃跑）
    PlayerTargetSelecting, // 玩家正在选择目标敌人 ← 添加这个
    PlayerActing,
    EnemyActing,
    CheckVictory,
    BattleOver
};

enum class PlayerAction {
    Attack,
    Skill,
    Item,
    Defend,
    Flee
};

class BattleSystem {
public:
    BattleSystem();

    void initBattle(MangoPtr<Player> player, MangoVector<MangoPtr<Enemy>> enemies);
    void startBattle();
    void endBattle();

    BattleStage getStage() const;

    void setSelectedEnemyIndex(size_t index);
    size_t getSelectedEnemyIndex() const;

    void submitPlayerAction(PlayerAction action);

    void advance(); // 推进到下一阶段

    bool isVictory() const;
    bool isDefeated() const;
    bool isBattleOver() const;

    bool isEnemyAlive(std::size_t index) const;

    void setDefending(bool value);
    bool isDefending() const;

    const MangoVector<MangoPtr<Enemy>>& getEnemies() const;

private:
    void performPlayerAttack();
    void performPlayerDefend();
    void performPlayerFlee();

    void performEnemyTurn();
    bool checkVictory();

private:
    MangoPtr<Player> m_player;
    MangoVector<MangoPtr<Enemy>> m_enemies;

    BattleStage m_stage = BattleStage::Init;
    BattleStage m_stageBeforeCheck = BattleStage::Init;

    PlayerAction m_pendingAction = PlayerAction::Defend;
    size_t m_selectedEnemyIndex = 0;
    size_t m_enemyIndex = 0;

    bool m_isDefending = false;
    bool m_battleEnded = false;
    bool m_victory = false;
};
