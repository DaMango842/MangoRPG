#include "BattleSystem.h"
#include "Random.hpp"
#include "Logger.h"

BattleSystem::BattleSystem() {}

void BattleSystem::initBattle(MangoPtr<Player> player, MangoVector<MangoPtr<Enemy>> enemies) {
    m_player = std::move(player);
    m_enemies = std::move(enemies);

    m_stage = BattleStage::Init;
    m_stageBeforeCheck = BattleStage::Init;

    m_battleEnded = false;
    m_victory = false;
    m_enemyIndex = 0;
    m_isDefending = false;
    m_pendingAction = PlayerAction::Defend;
}

void BattleSystem::startBattle() {
    m_stage = BattleStage::PlayerSelecting;
}

void BattleSystem::endBattle() {
    m_battleEnded = true;
    m_stage = BattleStage::BattleOver;
}

BattleStage BattleSystem::getStage() const {
    return m_stage;
}

void BattleSystem::setSelectedEnemyIndex(size_t index) {
    m_selectedEnemyIndex = index;
}

size_t BattleSystem::getSelectedEnemyIndex() const {
    return m_selectedEnemyIndex;
}

void BattleSystem::submitPlayerAction(PlayerAction action) {
    m_pendingAction = action;
    m_stage = BattleStage::PlayerActing;
}

void BattleSystem::advance() {
    switch (m_stage) {
    case BattleStage::Init:
        startBattle();
        break;

    case BattleStage::PlayerActing:
        switch (m_pendingAction) {
        case PlayerAction::Attack: performPlayerAttack(); break;
        case PlayerAction::Defend: performPlayerDefend(); break;
        case PlayerAction::Flee:   performPlayerFlee(); break;
        default: break;
        }
        m_stageBeforeCheck = BattleStage::PlayerActing;
        m_stage = BattleStage::CheckVictory;
        break;

    case BattleStage::EnemyActing:
        performEnemyTurn();
        m_stageBeforeCheck = BattleStage::EnemyActing;
        m_stage = BattleStage::CheckVictory;
        break;

    case BattleStage::CheckVictory:
        if (checkVictory()) {
            endBattle();
        }
        else {
            if (m_stageBeforeCheck == BattleStage::PlayerActing) {
                m_stage = BattleStage::EnemyActing;
            }
            else {
                m_stage = BattleStage::PlayerSelecting;
            }
        }
        break;

    case BattleStage::PlayerSelecting:
        // 等待 submitPlayerAction 外部驱动进入 PlayerActing
        break;

    case BattleStage::BattleOver:
        break;
    }
}

bool BattleSystem::checkVictory() {
    bool allDead = true;
    for (const auto& e : m_enemies) {
        if (e && e->getAttributes().get<int>("HP", 0) > 0)
            allDead = false;
    }

    bool playerDead = m_player->getAttributes().get<int>("HP", 0) <= 0;

    if (playerDead) {
        m_victory = false;
        return true;
    }
    if (allDead) {
        m_victory = true;
        return true;
    }

    return false;
}

bool BattleSystem::isVictory() const {
    return m_battleEnded && m_victory;
}

bool BattleSystem::isDefeated() const {
    return m_battleEnded && !m_victory;
}

bool BattleSystem::isBattleOver() const {
    return m_battleEnded;
}

bool BattleSystem::isEnemyAlive(std::size_t index) const
{
    return index >= 0 &&
        index < static_cast<int>(m_enemies.size()) &&
        m_enemies[index] &&
        m_enemies[index]->getAttributes().get<float>("HP") > 0.f;
}

void BattleSystem::setDefending(bool value) {
    m_isDefending = value;
}

bool BattleSystem::isDefending() const {
    return m_isDefending;
}

const MangoVector<MangoPtr<Enemy>>& BattleSystem::getEnemies() const {
    return m_enemies;
}

void BattleSystem::performPlayerAttack() {
    if (m_selectedEnemyIndex >= m_enemies.size()) return;
    auto& target = m_enemies[m_selectedEnemyIndex];
    if (!target) return;

    int atk = m_player->getAttributes().get<int>("ATK", 0);
    target->takeDamage(atk);

    LOG_DEBUG(std::format("{} 攻击了 {}!",
        toStdString(m_player->getName()),
        toStdString(target->getName())
    ));
}

void BattleSystem::performPlayerDefend() {
    m_player->getAttributes().add("DEF", 2.0f);
}

void BattleSystem::performPlayerFlee() {
    if (Random::chance(0.5f)) {
        m_victory = false;
        endBattle();
    }
}

void BattleSystem::performEnemyTurn() {
    for (const auto& e : m_enemies) {
        if (!e || e->getAttributes().get<int>("HP", 0) <= 0)
            continue;
        int atk = e->getAttributes().get<int>("ATK", 0);
        m_player->takeDamage(atk);
    }
}
