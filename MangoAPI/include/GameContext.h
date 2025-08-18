#pragma once

#include "Player.h"
#include "Factory.h"
#include "SafeTypes.hpp"

#include <MangoPtr.hpp>
#include <MangoVector.hpp>

struct GameData {
    size_t totalGameTime = 0;
    size_t battleWinCount = 0;
    size_t battleLoseCount = 0;
    size_t enemyKilledCount = 0;

    size_t uniqueItemCount = 0;            // 首次获得道具的数量
    size_t itemUsedCount = 0;              // 使用道具次数

    size_t skillUsedCount = 0;             // 技能使用次数
    size_t totalDamageDealt = 0;           // 累计造成的伤害
    size_t totalDamageTaken = 0;           // 累计受到的伤害

    size_t enemyKilledCount = 0;           // 击败敌人数
    size_t treasureChestOpened = 0;        // 打开宝箱次数

    size_t escapeSuccessCount = 0;         // 成功逃跑次数
    size_t escapeFailCount = 0;            // 逃跑失败次数

    size_t saveCount = 0;                  // 保存次数
    size_t saveLoadedCount = 0;            // 加载存档次数

    size_t routeCompletedCount = 0;        // 完成路线次数（WIP）
    size_t loopCount = 0;                  // 周目次数
    size_t achievementUnlockedCount = 0;   // 成就解锁数量

    size_t totalSteps = 0;                 // 累计移动步数
    size_t npcInteractionCount = 0;        // 与 NPC 的对话次数

    size_t dialogueSeenCount = 0;          // 累计触发对话次数
    size_t dialogueSkippedCount = 0;       // 累计跳过对话次数
};

struct GameContext {
    UniqueHandle<Player> playerData;
    MangoVector<MangoPtr<Item>> inventoryData;
    MangoVector<MangoPtr<Enemy>> enemiesList;
    MangoVector<MangoPtr<Enemy>> bossesList;
    MangoVector<MangoPtr<Weapon>> weaponsList;
    MangoVector<MangoPtr<Armor>> armorsList;
    MangoVector<MangoPtr<Consumable>> consumableList;
    //MangoVector<MangoPtr<KeyItem>> keyitemsList;

    GameContext() = default;

};
