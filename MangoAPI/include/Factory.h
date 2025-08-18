#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <SFML/System/String.hpp>

class Enemy;
class Item;
class Weapon;
class Armor;
class Consumable;

class Factory {
public:
    static Factory& get();

    void loadEnemies(const std::string& path = "Assets/Data/enemies.json");
    std::unique_ptr<Enemy> createEnemy(const sf::String& name);
    std::unique_ptr<Enemy> createEnemy(const std::string& name);
    const std::unordered_map<std::string, std::unique_ptr<Enemy>>& getEnemies() const;

    void loadWeapons(const std::string& path = "Assets/Data/weapons.json");
    std::unique_ptr<Weapon> createWeapon(const sf::String& name);
    std::unique_ptr<Weapon> createWeapon(const std::string& name);
    const std::unordered_map<std::string, std::unique_ptr<Weapon>>& getWeapons() const;

    void loadArmors(const std::string& path = "Assets/Data/armors.json");
    std::unique_ptr<Armor> createArmor(const sf::String& name);
    std::unique_ptr<Armor> createArmor(const std::string& name);
    const std::unordered_map<std::string, std::unique_ptr<Armor>>& getArmors() const;

    void loadConsumables(const std::string& path = "Assets/Data/consumables.json");
    std::unique_ptr<Consumable> createConsumable(const sf::String& name);
    std::unique_ptr<Consumable> createConsumable(const std::string& name);
    const std::unordered_map<std::string, std::unique_ptr<Consumable>>& getConsumables() const;

private:
    Factory() = default;

    std::unordered_map<std::string, std::unique_ptr<Enemy>> m_enemyPrototypes;
    std::unordered_map<std::string, std::unique_ptr<Weapon>> m_weaponPrototypes;
    std::unordered_map<std::string, std::unique_ptr<Armor>> m_armorPrototypes;
    std::unordered_map<std::string, std::unique_ptr<Consumable>> m_consumablePrototypes;
};
