#include "Player.h"
#include "TileMap.h"
#include <cmath>

#include "MangoException.h"
//#include "Logger.h"

#include "Random.hpp"

Player::Player()
{
    setImagePath("Assets/Image/Character/testChar.png");
}

Player::~Player() {}


void Player::initAnimator()
{
    if (!m_playerImgPath.empty()) {
        m_animation.loadTexture(m_playerImgPath);
    }
    else { 
        m_animation.loadTexture("Assets/Image/Character/testChar.png"); // 使用默认
    }

    m_animation.createAnimation("walk_down", 0, 0, 32, 48, 4);
    m_animation.createAnimation("walk_left", 0, 1, 32, 48, 4);
    m_animation.createAnimation("walk_right", 0, 2, 32, 48, 4);
    m_animation.createAnimation("walk_up", 0, 3, 32, 48, 4);
    m_animation.createAnimation("idle_down", 0, 0, 32, 48, 1);
    m_animation.createAnimation("idle_left", 0, 1, 32, 48, 1);
    m_animation.createAnimation("idle_right", 0, 2, 32, 48, 1);
    m_animation.createAnimation("idle_up", 0, 3, 32, 48, 1);

    m_animation.setAnimation("idle_down");
    m_animation.setAnimationSpeed(10.f);

    m_position = { 400.f, 300.f };
    m_targetPosition = m_position;
    m_animation.setPosition(m_position);
}

void Player::setTileMap(TileMap* tileMap)
{
    m_tileMap = tileMap;
}

void Player::update(float deltaTime)
{
    if (m_isMoving)
    {
        sf::Vector2f direction = m_targetPosition - m_position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < m_speed * deltaTime)
        {
            m_position = m_targetPosition;
            m_isMoving = false;

            // 停止动画为 idle
            std::string idleAnim = "idle_" + directionToString(m_lastDirection);
            m_animation.setAnimation(idleAnim);
            m_currentAnimName = idleAnim;
        }
        else
        {
            direction /= distance;
            m_position += direction * m_speed * deltaTime;
        }

        m_animation.setPosition(m_position);
        m_animation.update(deltaTime);
        return;
    }

    // 不是正在移动中时才接受新输入（单步一格）
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) tryMove(Direction::Up);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) tryMove(Direction::Down);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) tryMove(Direction::Left);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) tryMove(Direction::Right);

    m_animation.setPosition(m_position);
    m_animation.update(deltaTime);
}

bool Player::isAlive() const {
    return getAttributes().get<int>("HP") > 0;
}

void Player::takeDamage(int baseDamage)
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
        toStdString(m_playerName), finalDamage, currentHP, newHP));
}

MangoPtr<Player> Player::createDefaultData()
{
    // 使用工厂函数创建有效的 Player 对象
    auto p = make_mango_ptr<Player>();

    // 检查是否创建成功
    if (!p) {
        // 处理创建失败（如内存不足）
        // 可以抛出异常或返回空指针
        return nullptr;
    }

    // 设置属性
    p->setName(L"Player");
    p->m_attributes.set("LV", 1);
    p->m_attributes.set("LVMAX", 10);
    p->m_attributes.set("HP", 100);
    p->m_attributes.set("HPMAX", 100);
    p->m_attributes.set("MANA", 0);
    p->m_attributes.set("MANAMAX", 0);
    p->m_attributes.set("ATK", 4);
    p->m_attributes.set("DEF", 2);
    p->m_attributes.set("MATK", 0);
    p->m_attributes.set("MDEF", 0);
    p->m_attributes.set("MONEY", 0);
    p->m_attributes.set("EXP", 0);
    p->m_attributes.set("NEXTEXP", 10);

    return p;
}

void Player::LevelUp()
{
    int exp = m_attributes.get<int>("EXP");
    int nextExp = m_attributes.get<int>("NEXTEXP");
    int level = m_attributes.get<int>("LEVEL");

    while (exp >= nextExp) {
        exp -= nextExp;
        level += 1;

        // 升级成长
        int hp = m_attributes.get<int>("HP") + 10;
        int mp = m_attributes.get<int>("MP") + 5;
        int atk = m_attributes.get<int>("ATK") + 2;
        int def = m_attributes.get<int>("DEF") + 2;
        int matk = m_attributes.get<int>("MATK") + (level >= 10 ? 1 : 0);
        int mdef = m_attributes.get<int>("MDEF") + (level >= 10 ? 1 : 0);

        nextExp += 50;

        // 更新属性
        m_attributes.set("HP", hp);
        m_attributes.set("MP", mp);
        m_attributes.set("ATK", atk);
        m_attributes.set("DEF", def);
        m_attributes.set("MATK", matk);
        m_attributes.set("MDEF", mdef);
        
        m_attributes.set("NEXTEXP", nextExp);
        m_attributes.set("LEVEL", level);

        LOG_INFO(std::format("升级到 {} 级！", level));
    }

    m_attributes.set("EXP", exp);
}

void Player::tryMove(Direction dir)
{
    if (!m_tileMap) return;

    float tileW = static_cast<float>(m_tileMap->getTileWidth());
    float tileH = static_cast<float>(m_tileMap->getTileHeight());

    sf::Vector2f offset(0.f, 0.f);
    switch (dir)
    {
    case Direction::Up:    offset.y = -tileH; break;
    case Direction::Down:  offset.y = tileH; break;
    case Direction::Left:  offset.x = -tileW; break;
    case Direction::Right: offset.x = tileW; break;
    }

    sf::Vector2f newTarget = m_position + offset;

    size_t tileX = static_cast<size_t>(newTarget.x) / m_tileMap->getTileWidth();
    size_t tileY = static_cast<size_t>(newTarget.y) / m_tileMap->getTileHeight();

    if (!m_tileMap->isSolidTileAt(tileX, tileY))
    {
        m_targetPosition = newTarget;
        m_isMoving = true;
        m_lastDirection = dir;

        std::string animName = "walk_" + directionToString(dir);
        if (animName != m_currentAnimName)
        {
            m_animation.setAnimation(animName);
            m_currentAnimName = animName;
        }
    }
}

std::string Player::directionToString(Direction dir) const
{
    switch (dir)
    {
    case Direction::Up:    return "up";
    case Direction::Down:  return "down";
    case Direction::Left:  return "left";
    case Direction::Right: return "right";
    default: return "down";
    }
}

void Player::render(sf::RenderTarget& target)
{
    m_animation.render(target);
}

