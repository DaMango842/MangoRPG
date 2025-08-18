#include "Armor.h"

Armor::Armor(const sf::String& name, ArmorType type)
	: Item(name, ItemType::Armor), m_armorType(type)
{
}

void Armor::onEquip()
{
}

void Armor::onUnequip()
{
}

