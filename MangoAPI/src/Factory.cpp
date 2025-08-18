#include "Factory.h"
#include "Enemy.h"
#include "Item.h"
#include "Weapon.h"
#include "Armor.h"
#include "Consumable.h"
#include "Logger.h"
#include "Utils.h"

#include <fstream>
#include <nlohmann/json.hpp>

Factory& Factory::get() {
    static Factory instance;
    return instance;
}

// -----------------------------
// Enemy
// -----------------------------

void Factory::loadEnemies(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open enemy file: " + path);
        return;
    }

    nlohmann::json data;
    try {
        file >> data;
    }
    catch (const std::exception& e) {
        LOG_ERROR("JSON parse error in " + path + ": " + e.what());
        return;
    }

    size_t count = 0;
    for (const auto& entry : data) {
        std::string utf8Name = entry.value("Name", "");
        if (utf8Name.empty()) {
            LOG_WARN("Enemy entry missing Name field, skipped.");
            continue;
        }

        sf::String sfName = sf::String::fromUtf8(utf8Name.begin(), utf8Name.end());

        auto enemy = std::make_unique<Enemy>();
        enemy->setName(sfName);
        enemy->setPosition({ 100.f, 100.f });
        enemy->getAttributes().fromJson(entry["Attributes"]);

        m_enemyPrototypes[utf8Name] = std::move(enemy);  // 键使用 UTF-8 原文
        ++count;
    }

    LOG_INFO("Loaded " + std::to_string(count) + " enemy prototype(s) from " + path);
}

std::unique_ptr<Enemy> Factory::createEnemy(const std::string& name) {
    auto it = m_enemyPrototypes.find(name);
    if (it != m_enemyPrototypes.end()) {
        return std::make_unique<Enemy>(*it->second);  // 通过拷贝创建新对象
    }
    LOG_WARN("Enemy prototype not found: " + name);
    return nullptr;
}

std::unique_ptr<Enemy> Factory::createEnemy(const sf::String& name) {
    return createEnemy(toStdString(name));
}

const std::unordered_map<std::string, std::unique_ptr<Enemy>>& Factory::getEnemies() const {
    return m_enemyPrototypes;
}

// -----------------------------
// Weapon
// -----------------------------

void Factory::loadWeapons(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open weapon file: " + path);
        return;
    }

    nlohmann::json data;
    try {
        file >> data;
    }
    catch (const std::exception& e) {
        LOG_ERROR("JSON parse error in " + path + ": " + e.what());
        return;
    }

    size_t count = 0;
    for (const auto& entry : data) {
        std::string name = entry.value("name", "");
        if (name.empty()) {
            LOG_WARN("Weapon entry missing name field, skipped.");
            continue;
        }

        std::string wtypeStr = entry.value("w_type", "None");
        WeaponType wtype = WeaponType::None;
        if (wtypeStr == "sword") wtype = WeaponType::Sword;
        else if (wtypeStr == "greatsword") wtype = WeaponType::Greatsword;
        else if (wtypeStr == "dagger") wtype = WeaponType::Dagger;

        sf::String sfName = sf::String::fromUtf8(name.begin(), name.end());
        auto weapon = std::make_unique<Weapon>(sfName, wtype);

        if (entry.contains("desc")) {
            auto descStr = entry["desc"].get<std::string>();
            weapon->setDescription(sf::String::fromUtf8(descStr.begin(), descStr.end()));
        }

        m_weaponPrototypes[name] = std::move(weapon);
        ++count;
    }

    LOG_INFO("Loaded " + std::to_string(count) + " weapon prototype(s) from " + path);
}

std::unique_ptr<Weapon> Factory::createWeapon(const std::string& name) {
    auto it = m_weaponPrototypes.find(name);
    if (it != m_weaponPrototypes.end()) {
        return std::make_unique<Weapon>(*it->second);
    }
    LOG_WARN("Weapon prototype not found: " + name);
    return nullptr;
}

std::unique_ptr<Weapon> Factory::createWeapon(const sf::String& name) {
    return createWeapon(toStdString(name));
}

const std::unordered_map<std::string, std::unique_ptr<Weapon>>& Factory::getWeapons() const {
    return m_weaponPrototypes;
}

// -----------------------------
// Armor
// -----------------------------

void Factory::loadArmors(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open armor file: " + path);
        return;
    }

    nlohmann::json data;
    try {
        file >> data;
    }
    catch (const std::exception& e) {
        LOG_ERROR("JSON parse error in " + path + ": " + e.what());
        return;
    }

    size_t count = 0;
    for (const auto& entry : data) {
        std::string name = entry.value("name", "");
        if (name.empty()) {
            LOG_WARN("Armor entry missing name field, skipped.");
            continue;
        }

        std::string atypeStr = entry.value("a_type", "None");
        ArmorType atype = ArmorType::None;
        if (atypeStr == "helmet") atype = ArmorType::Helmet;
        else if (atypeStr == "chestplate") atype = ArmorType::Chestplate;

        sf::String sfName = sf::String::fromUtf8(name.begin(), name.end());
        auto armor = std::make_unique<Armor>(sfName, atype);

        if (entry.contains("desc")) {
            auto descStr = entry["desc"].get<std::string>();
            armor->setDescription(sf::String::fromUtf8(descStr.begin(), descStr.end()));
        }

        m_armorPrototypes[name] = std::move(armor);
        ++count;
    }

    LOG_INFO("Loaded " + std::to_string(count) + " armor prototype(s) from " + path);
}

std::unique_ptr<Armor> Factory::createArmor(const std::string& name) {
    auto it = m_armorPrototypes.find(name);
    if (it != m_armorPrototypes.end()) {
        return std::make_unique<Armor>(*it->second);
    }
    LOG_WARN("Armor prototype not found: " + name);
    return nullptr;
}

std::unique_ptr<Armor> Factory::createArmor(const sf::String& name) {
    return createArmor(toStdString(name));
}

const std::unordered_map<std::string, std::unique_ptr<Armor>>& Factory::getArmors() const {
    return m_armorPrototypes;
}

// -----------------------------
// Consumable
// -----------------------------

void Factory::loadConsumables(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open consumable file: " + path);
        return;
    }

    nlohmann::json data;
    try {
        file >> data;
    }
    catch (const std::exception& e) {
        LOG_ERROR("JSON parse error in " + path + ": " + e.what());
        return;
    }

    size_t count = 0;
    for (const auto& entry : data) {
        std::string name = entry.value("name", "");
        if (name.empty()) {
            LOG_WARN("Consumable entry missing name field, skipped.");
            continue;
        }

        std::string ctypeStr = entry.value("c_type", "None");
        ConsumableType ctype = ConsumableType::None;
        if (ctypeStr == "heal") ctype = ConsumableType::Heal;
        else if (ctypeStr == "food") ctype = ConsumableType::Food;
        else if (ctypeStr == "buff") ctype = ConsumableType::Buff;
        else if (ctypeStr == "debuff") ctype = ConsumableType::Debuff;

        sf::String sfName = sf::String::fromUtf8(name.begin(), name.end());
        auto consumable = std::make_unique<Consumable>(sfName, ctype);

        if (entry.contains("desc")) {
            auto descStr = entry["desc"].get<std::string>();
            consumable->setDescription(sf::String::fromUtf8(descStr.begin(), descStr.end()));
        }

        m_consumablePrototypes[name] = std::move(consumable);
        ++count;
    }

    LOG_INFO("Loaded " + std::to_string(count) + " consumable prototype(s) from " + path);
}

std::unique_ptr<Consumable> Factory::createConsumable(const std::string& name) {
    auto it = m_consumablePrototypes.find(name);
    if (it != m_consumablePrototypes.end()) {
        return std::make_unique<Consumable>(*it->second);
    }
    LOG_WARN("Consumable prototype not found: " + name);
    return nullptr;
}

std::unique_ptr<Consumable> Factory::createConsumable(const sf::String& name) {
    return createConsumable(toStdString(name));
}

const std::unordered_map<std::string, std::unique_ptr<Consumable>>& Factory::getConsumables() const {
    return m_consumablePrototypes;
}
