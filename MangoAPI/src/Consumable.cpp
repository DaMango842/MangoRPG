#include "Consumable.h"

Consumable::Consumable(const sf::String& name, ConsumableType consumableType)
    : Item(name, ItemType::Consumable),
    m_consumableType(consumableType)
{
    m_maxStack = 99;
}

void Consumable::onUse()
{

}
