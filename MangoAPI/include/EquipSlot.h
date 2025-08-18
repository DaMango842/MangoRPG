#pragma once
#include <memory>
#include <iostream>
#include <type_traits>
#include "Item.h"
#include "Weapon.h"
#include "Armor.h"

// 装备槽类型定义
enum class EquipSlotType {
    None = 0,
    MainHand,
    OffHand,
    Head,
    Chest,
    Legs,
    Feet,
    Shield,
    Accessory1,
    Accessory2
};

// 装备槽模板
template<typename T>
class EquipSlot {
    static_assert(std::is_base_of<Item, T>::value, "T must derive from Item");

public:
    explicit EquipSlot(EquipSlotType slotType) : m_slotType(slotType) {}

    bool equip(std::shared_ptr<Item> item) {
        if (!item || !isTypeCompatible(item->getType())) return false;
        if (!canEquipToSlot(item)) return false;

        auto casted = std::dynamic_pointer_cast<T>(item);
        if (!casted) return false;

        if (m_item) m_item->onUnequip();

        m_item = casted;
        m_item->onEquip();
        return true;
    }

    void unequip() {
        if (m_item) {
            m_item->onUnequip();
            m_item.reset();
        }
    }

    bool isEmpty() const {
        return !m_item;
    }

    std::shared_ptr<T> get() const {
        return m_item;
    }

    EquipSlotType getSlotType() const {
        return m_slotType;
    }

private:
    bool isTypeCompatible(ItemType type) const {
        if constexpr (std::is_same_v<T, Weapon>) {
            return type == ItemType::Weapon;
        }
        else if constexpr (std::is_same_v<T, Armor>) {
            return type == ItemType::Armor;
        }
        else {
            return false;
        }
    }

    bool canEquipToSlot(std::shared_ptr<Item> item) const {
        if constexpr (std::is_same_v<T, Weapon>) {
            // 武器只能装备主手或副手
            return m_slotType == EquipSlotType::MainHand || m_slotType == EquipSlotType::OffHand;
        }
        else if constexpr (std::is_same_v<T, Armor>) {
            auto armor = std::dynamic_pointer_cast<Armor>(item);
            if (!armor) return false;

            switch (armor->getArmorType()) {
            case ArmorType::Helmet:    return m_slotType == EquipSlotType::Head;
            case ArmorType::Chestplate:return m_slotType == EquipSlotType::Chest;
            //case ArmorType::Gloves:    return m_slotType == EquipSlotType::Feet;
            case ArmorType::Leggings:  return m_slotType == EquipSlotType::Legs;
            case ArmorType::Boots:     return m_slotType == EquipSlotType::Feet;
            case ArmorType::Shield:    return m_slotType == EquipSlotType::Shield;
            default:                   return false;
            }
        }
        return false;
    }

    EquipSlotType m_slotType;
    std::shared_ptr<T> m_item;
};
