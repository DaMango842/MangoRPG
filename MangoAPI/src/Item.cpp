#include "Item.h"

Item::Item(const sf::String& name, ItemType type)
    : m_name(name), m_type(type)
{
    switch (type)
    {
    case ItemType::Weapon:
    case ItemType::Armor:
        m_maxStack = 1;
        break;
    default:
        m_maxStack = 99;
        break;
    }
}

bool Item::canUse() const
{
    // 默认只有消耗品可用
    return (m_type == ItemType::Consumable);
}

void Item::onUse()
{
    // 默认空实现
}

bool Item::canEquip() const
{
    return (m_type == ItemType::Weapon || m_type == ItemType::Armor);
}

void Item::onEquip()
{
    // 默认空实现
}

void Item::onUnequip()
{
    // 默认空实现
}

bool Item::isStackable() const
{
    return m_maxStack > 1;
}

size_t Item::getMaxStack() const
{
    return m_maxStack;
}

sf::String Item::getDescription() const
{
    return m_description;
}

void Item::setDescription(const sf::String& text)
{
    m_description = text;
}
