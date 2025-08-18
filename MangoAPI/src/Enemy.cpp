#include "Enemy.h"
#include "Logger.h"
#include <MangoException.h>

#include <Random.hpp>

Enemy::Enemy(const std::string& texturePath)
{
    if (m_texture.loadFromFile(texturePath)) {
        m_sprite.setTexture(m_texture);

        sf::FloatRect bounds = m_sprite.getLocalBounds();
        m_sprite.setOrigin(bounds.width / 2.f, bounds.height);

        m_sprite.setPosition(100.f, 100.f);
        m_hasTexture = true;
    }
    else {
        m_placeholder.setSize({ 64.f, 64.f });
        m_placeholder.setFillColor(sf::Color::Red);
        m_placeholder.setOutlineColor(sf::Color::Black);
        m_placeholder.setOutlineThickness(2.f);
        m_placeholder.setPosition(100.f, 100.f);
        LOG_WARN("Enemy texture not found: " + texturePath);
    }
}

bool Enemy::loadFromJson(const nlohmann::json& j)
{
    // 加载名字（可选）
    if (j.contains("Name") && j["Name"].is_string()) {
        const std::string& utf8Name = j["Name"];
        m_name = sf::String::fromUtf8(utf8Name.begin(), utf8Name.end());  // ✅ 转换为 sf::String
    }

    // 加载贴图（可选）
    std::string texturePath = "Assets/Textures/enemy/static_enemy.png"; // 默认路径
    if (j.contains("Texture") && j["Texture"].is_string())
        texturePath = j["Texture"].get<std::string>();

    if (m_texture.loadFromFile(texturePath)) {
        m_sprite.setTexture(m_texture);

        sf::FloatRect bounds = m_sprite.getLocalBounds();
        m_sprite.setOrigin(bounds.width / 2.f, bounds.height);

        m_sprite.setPosition(100.f, 100.f);
        m_hasTexture = true;
    }
    else {
        m_hasTexture = false;
        m_placeholder.setSize({ 64.f, 64.f });
        m_placeholder.setFillColor(sf::Color::Red);
        m_placeholder.setOutlineColor(sf::Color::Black);
        m_placeholder.setOutlineThickness(2.f);
        m_placeholder.setPosition(100.f, 100.f);
        LOG_WARN("Failed to load enemy texture: " + texturePath);
    }

    if (j.contains("Attributes") && j["Attributes"].is_object()) {
        m_attributes.fromJson(j["Attributes"]);
    }

    return true;
}

bool Enemy::isAlive() const {
    return getAttributes().get<int>("HP") > 0;
}

void Enemy::takeDamage(int baseDamage)
{
    if (m_attributes.empty()) {
        throw AttributeException("Attribute has not been initialized!");
    }

    // 计算最终伤害（基础伤害 + 随机浮动）
    int randomBonus = Random::randint(0, 20);
    int finalDamage = baseDamage + randomBonus;

    // 扣血处理
    int currentHP = m_attributes.get<int>("HP");
    int newHP = std::max(0, currentHP - finalDamage);

    // 更新属性
    m_attributes.set("HP", newHP);

    // 输出日志
    LOG_DEBUG(std::format("{} 受到了 {} 点伤害!（当前HP: {} → {}）",
        toStdString(m_name), finalDamage, currentHP, newHP));
}


void Enemy::update(float) {}

void Enemy::render(sf::RenderTarget& target)
{
    if (m_hasTexture)
        target.draw(m_sprite);
    else
        target.draw(m_placeholder);
}

const sf::Vector2f& Enemy::getPosition() const {
    return m_hasTexture ? m_sprite.getPosition() : m_placeholder.getPosition();
}

void Enemy::setPosition(const sf::Vector2f& pos) {
    if (m_hasTexture)
        m_sprite.setPosition(pos);
    else
        m_placeholder.setPosition(pos);
}
