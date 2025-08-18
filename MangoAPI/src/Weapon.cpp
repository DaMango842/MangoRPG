#include "Weapon.h"

Weapon::Weapon(const sf::String& name, WeaponType weaponType)
    : Item(name, ItemType::Weapon), m_weaponType(weaponType)
{
}

void Weapon::onEquip()
{

}

void Weapon::onUnequip()
{

}

