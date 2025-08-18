#pragma once

#include "Item.h"

enum class WeaponType
{
    None = 0,
    Sword,        // 单手剑
    Greatsword,   // 双手剑
    Dagger,       // 匕首
    Axe,          // 斧头
    Spear,        // 长枪
    Bow,          // 弓
    Staff         // 法杖
};

class Weapon : public Item
{
public:
    Weapon(const sf::String& name, WeaponType weaponType);

    WeaponType getWeaponType() const { return m_weaponType; }

    // 可以重载 onEquip / onUse 等
    void onEquip() override;
    void onUnequip() override;
    //void onUse() override;

private:
    WeaponType m_weaponType = WeaponType::None;

};


