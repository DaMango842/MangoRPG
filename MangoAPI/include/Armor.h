#pragma once

#include "Item.h"

enum class ArmorType
{
    None = 0,
    Helmet,
    Chestplate,
    Gloves,
    Leggings,
    Boots,
    Shield
};

class Armor : public Item
{
public:
    Armor(const sf::String& name, ArmorType type = ArmorType::None);

    ArmorType getArmorType() const { return m_armorType; }

    // 装备时调用
    virtual void onEquip() override;

    // 卸下时调用
    virtual void onUnequip() override;


private:
    ArmorType m_armorType;
};
