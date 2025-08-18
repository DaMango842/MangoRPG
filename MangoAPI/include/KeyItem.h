#pragma once

#include "Item.h"

enum class KeyItemType {
    None = 0,
    Quest,     // 任务触发用
    Event,     // 剧情流程用
    System,    // 游戏机制（如传送石）
    Unknown = 999
};

class KeyItem : public Item
{
public:
    KeyItem(const sf::String& name, KeyItemType keyItemType);

    KeyItemType getKeyItemType() const { return m_keyItemType; }

    // 不提供任何与装备与使用相关的接口

private:
    KeyItemType m_keyItemType;
};
