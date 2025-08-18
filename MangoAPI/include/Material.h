#pragma once

#include "Item.h"

enum class MaterialType
{
    None = 0,
    Wood,
    Stone,
    Gem,
    Unknown = 999
};

class Material : public Item
{
public:
    Material(const sf::String& name, MaterialType materialType);

    MaterialType getMaterialType() const { return m_materialType; }

    // 与KeyItem同理,不提供装备和使用物品相关的接口

private:
    MaterialType m_materialType;
};
