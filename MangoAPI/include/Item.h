#pragma once
#include <string>
#include <memory>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include "Attribute.h"

enum class ItemType
{
    None = 0,
    Weapon,
    Armor,
    Consumable,
    Material,
    KeyItem,
    Misc,
    Unknown = 999
};

struct Price {
    int buy = 0;
    int sell = 0;

    void fromJson(const nlohmann::json& j) {
        if (j.is_array()) {
            if (j.size() > 0) buy = j[0].get<int>();
            if (j.size() > 1) sell = j[1].get<int>();
        }
    }
};

class Item
{
public:
    Item(const sf::String& name, ItemType type = ItemType::Unknown);
    virtual ~Item() = default;

    int getID() const { return m_id; }
    const sf::String& getName() const { return m_name; }
    ItemType getType() const { return m_type; }

    // 使用接口
    virtual bool canUse() const;
    virtual void onUse();

    // 装备接口
    virtual bool canEquip() const;
    virtual void onEquip();
    virtual void onUnequip();

    // 堆叠接口
    virtual bool isStackable() const;
    virtual size_t getMaxStack() const;

    // 物品描述
    virtual sf::String getDescription() const;
    virtual void setDescription(const sf::String& text);

    Price getPrice() const { return m_price; }
    void setPrice(int buy, int sell) { m_price = { buy, sell }; }

    // 属性相关接口
    /*bool hasAttributes() const { return m_attributes != nullptr; }*/
    const Attribute& getAttributes() const { return m_attributes; }
    Attribute& getAttributes() { return m_attributes; }

protected:
    int m_id;
    sf::String m_name;
    sf::String m_description;
    ItemType m_type;
    size_t m_maxStack = 999;
    Price m_price;

    Attribute m_attributes; 
};
