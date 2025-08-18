#pragma once
#include <SFML/Graphics.hpp>
#include "BaseObject.h"
#include "SpriteAnimation.h"

#include "Attribute.h"

#include "Logger.h"

#include <MangoPtr.hpp>

class TileMap;

class Player : public BaseObject
{
public:
    Player();
    virtual ~Player();

    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void initAnimator();
    void setImagePath(const std::string fileName) { m_playerImgPath = fileName; }
    std::string getImagePath() { return m_playerImgPath; }
    const std::string& getImagePath() const { return m_playerImgPath; }


    void setTileMap(TileMap* tileMap);
    SpriteAnimation& getAnimator() { return m_animation; }

    const sf::Vector2f& getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& pos) { m_position = pos; }

    Attribute& getAttributes() { return m_attributes; }
    const Attribute& getAttributes() const { return m_attributes; }

    void setName(const sf::String& name) { m_playerName = name; }
    const sf::String& getName() const { return m_playerName; }


    void takeDamage(int dmg);

    bool isAlive() const;

    static MangoPtr<Player> createDefaultData();

    
    void LevelUp();

private:
    void tryMove(Direction dir);
    std::string directionToString(Direction dir) const;

    SpriteAnimation m_animation;

    sf::Vector2f m_position;        // 当前坐标（像素）
    sf::Vector2f m_targetPosition;  // 移动目标位置（像素）

    bool m_isMoving = false;
    float m_speed = 100.f; // 每秒移动像素

    Direction m_lastDirection = Direction::Down;
    std::string m_currentAnimName = "idle_down";

    TileMap* m_tileMap = nullptr;

    sf::String m_playerName;
    Attribute m_attributes;

    std::string m_playerImgPath;

    ENABLE_LOG_INJECTION(Player);
};
