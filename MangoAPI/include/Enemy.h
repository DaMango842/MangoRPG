#pragma once

#include "BaseObject.h"
#include "Attribute.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <nlohmann/json.hpp>

class Enemy : public BaseObject
{
public:
    explicit Enemy(const std::string& texturePath = "Assets/Image/Enemies/enemy_placeholder.png");

    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    bool hasTexture() const { return m_hasTexture; }
    const sf::Vector2f& getPosition() const;
    void setPosition(const sf::Vector2f& pos);

    void setSprite(const sf::Sprite& spr) { m_sprite = spr; }
    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

    bool loadFromJson(const nlohmann::json& j);

    const Attribute& getAttributes() const { return m_attributes; }
    Attribute& getAttributes() { return m_attributes; }

    void setName(const sf::String& name) { m_name = name; }
    const sf::String& getName() const { return m_name; }

    void takeDamage(int damage);

    bool isAlive() const;

private:
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    bool m_hasTexture = false;

    sf::RectangleShape m_placeholder;
    Attribute m_attributes;

    sf::String m_name;  
};
