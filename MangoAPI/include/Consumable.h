#pragma once

#include "Item.h"

// 消耗品细分类别
enum class ConsumableType
{
    None = 0,
    Heal,
    Food,
    Scroll,
    Buff,
    Debuff,
    Unknown = 999
};

class Consumable : public Item
{
public:
    Consumable(const sf::String& name, ConsumableType consumableType);

    ConsumableType getConsumableType() const { return m_consumableType; }

    void onUse() override;

private:
    ConsumableType m_consumableType;
};
