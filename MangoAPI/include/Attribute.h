#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <concepts> // C++20 concepts
#include <nlohmann/json.hpp>
#include <SFML/System/String.hpp>
#include "Translator.h"

/**
 * @brief 属性名称映射表
 *
 * 将属性标识符映射到本地化的显示名称
 *
 * 注意：TR() 是翻译宏，用于获取本地化字符串
 */
inline const std::unordered_map<std::string, sf::String> attributeName = {
    {"LV", TR("STATUS.LV")},         // 当前等级
    {"LVMAX", L"等级上限"},          // 等级上限
    {"HP", TR("STATUS.HP")},         // 当前生命值
    {"HPMAX", L"血量上限"},          // 生命值上限
    {"MANA", TR("STATUS.MP")},       // 当前法力值
    {"MANAMAX", L"法力上限"},        // 法力值上限
    {"ATK", TR("STATUS.ATK")},       // 物理攻击力
    {"DEF", TR("STATUS.DEF")},       // 物理防御力
    {"MATK", TR("STATUS.MATK")},     // 魔法攻击力
    {"MDEF", TR("STATUS.MDEF")},     // 魔法防御力
    {"MONEY", TR("STATUS.MONEY")},   // 金钱
    {"EXP", TR("STATUS.EXP")},       // 当前经验值
    {"NEXTEXP", TR("STATUS.NEXTEXP")} // 升级所需经验值
};

/**
 * @brief 属性显示顺序
 *
 * 定义属性在UI中显示的顺序
 */
static const std::vector<std::string> attributeDisplayOrder = {
    "LV", "HP", "MANA", "ATK", "DEF", "MATK", "MDEF", "EXP", "NEXTEXP", "MONEY"
};

/**
 * @class Attribute
 * @brief 角色属性管理系统
 *
 * 提供角色属性的存储、访问、序列化和显示功能
 *
 * 主要功能：
 * 1. 存储各种数值属性（生命值、攻击力等）
 * 2. 支持属性值的设置、获取和修改
 * 3. 生成格式化的属性显示字符串
 * 4. 支持JSON格式的序列化和反序列化
 * 5. 提供空状态检查
 *
 * 使用示例：
 * @code
 *   Attribute attr;
 *   attr.set("HP", 100);
 *   attr.set("ATK", 25.5);
 *
 *   int hp = attr.get<int>("HP", 0); // 获取HP值
 *   attr.add("ATK", 5.0); // 增加攻击力
 *
 *   sf::String display = attr.getDisplayString("HP"); // "HP: 100/100"
 * @endcode
 */
class Attribute 
{
public:
    /**
     * @brief 设置属性值
     *
     * 如果属性已存在则更新，否则创建新属性
     *
     * @tparam T 数值类型（int, float, double等）
     * @param name 属性名称（如 "HP"、"ATK"）
     * @param value 属性数值
     */
    template<typename T>
        requires std::is_arithmetic_v<T>
    void set(const std::string& name, T value) {
        m_attributes[name] = static_cast<double>(value);
    }

    /**
     * @brief 获取属性值
     *
     * 模板方法，支持获取任意数值类型的属性值
     *
     * @tparam T 返回类型（必须为数值类型）
     * @param name 属性名称
     * @param defaultValue 属性不存在时返回的默认值
     * @return T 属性值（若不存在则返回默认值）
     *
     * 示例：
     * @code
     *   int level = attr.get<int>("LV", 1);
     *   float attack = attr.get<float>("ATK", 10.0f);
     *   double defense = attr.get<double>("DEF", 5.5);
     * @endcode
     */
    template<typename T>
        requires std::is_arithmetic_v<T>
    T get(const std::string& name, T defaultValue = T{}) const {
        auto it = m_attributes.find(name);
        if (it == m_attributes.end())
            return defaultValue;
        return static_cast<T>(it->second);
    }

    /**
     * @brief 修改属性值（增加或减少）
     *
     * @tparam T 数值类型（int, float, double等）
     * @param name 属性名称
     * @param delta 要增加/减少的数值（正数为增加，负数为减少）
     */
    template<typename T>
        requires std::is_arithmetic_v<T>
    void add(const std::string& name, T delta) {
        m_attributes[name] += static_cast<double>(delta);
    }

    /**
     * @brief 检查属性是否存在
     *
     * @param name 属性名称
     * @return true 属性存在
     * @return false 属性不存在
     */
    bool has(const std::string& name) const {
        return m_attributes.find(name) != m_attributes.end();
    }

    /**
     * @brief 获取所有属性的引用
     *
     * @return const std::unordered_map<std::string, double>& 属性映射表
     */
    const std::unordered_map<std::string, double>& getAll() const {
        return m_attributes;
    }

    /**
     * @brief 获取格式化的属性显示字符串
     *
     * 根据属性类型生成不同的显示格式：
     * 1. 有对应上限的属性（如HP/HPMAX）显示为 "当前值/最大值"
     * 2. 其他属性显示为 "属性名: 值"
     *
     * @param name 属性名称
     * @return sf::String 格式化后的显示字符串（支持宽字符）
     */
    sf::String getDisplayString(const std::string& name) const {
        std::wostringstream oss;
        auto& translator = Translator::get();

        // 特殊处理有上限值的属性
        if (name == "LV" && has("LVMAX")) {
            oss << translator.tr("STATUS.LV").toWideString() << L"："
                << get<int>("LV") << L" / " << get<int>("LVMAX");
            return sf::String(oss.str());
        }
        if (name == "HP" && has("HPMAX")) {
            oss << translator.tr("STATUS.HP").toWideString() << L"："
                << get<int>("HP") << L" / " << get<int>("HPMAX");
            return sf::String(oss.str());
        }
        if (name == "MANA" && has("MANAMAX")) {
            oss << translator.tr("STATUS.MP").toWideString() << L"："
                << get<int>("MANA") << L" / " << get<int>("MANAMAX");
            return sf::String(oss.str());
        }

        // 普通属性处理
        double value = get<double>(name);
        auto it = attributeName.find(name);
        if (it != attributeName.end()) {
            // 使用本地化名称
            oss << it->second.toWideString() << L"：" << static_cast<int>(value);
        }
        else {
            // 未知属性使用原始名称
            oss << sf::String::fromUtf8(name.begin(), name.end()).toWideString()
                << L"：" << static_cast<int>(value);
        }
        return sf::String(oss.str());
    }

    /**
     * @brief 将属性转换为JSON格式
     *
     * @return nlohmann::json 包含所有属性的JSON对象
     */
    nlohmann::json toJson() const {
        nlohmann::json j;
        for (const auto& [key, value] : m_attributes) {
            j[key] = value;
        }
        return j;
    }

    /**
     * @brief 从JSON加载属性
     *
     * 清空当前属性并从JSON对象加载
     *
     * @param j 包含属性的JSON对象
     */
    void fromJson(const nlohmann::json& j) {
        m_attributes.clear();
        for (auto& [key, val] : j.items()) {
            if (val.is_number()) {
                m_attributes[key] = val.get<double>();
            }
        }
    }

    /**
     * @brief 检查属性是否为空
     *
     * @return true 没有存储任何属性
     * @return false 至少有一个属性
     */
    explicit operator bool() const {
        return !m_attributes.empty();
    }

    /**
     * @brief 检查属性是否为空
     *
     * @return true 没有存储任何属性
     * @return false 至少有一个属性
     */
    bool empty() const {
        return m_attributes.empty();
    }

private:
    /// @brief 属性存储容器
    /// 使用unordered_map存储属性名到数值的映射
    /// 内部统一使用double类型存储以保证精度
    std::unordered_map<std::string, double> m_attributes;
};
