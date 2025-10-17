#pragma once

#include "Animation.h"

#include <unordered_map>

#include <string>


// SpriteAnimation
class SpriteAnimation : public Animation
{
public:
    SpriteAnimation() {}
    virtual ~SpriteAnimation() {}

    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    bool loadTexture(const std::string& path);

    void setPosition(const sf::Vector2f& pos);
    void setPosition(float x, float y);

    const std::vector<sf::IntRect>& createAnimation(const std::string& anim_name, size_t x, size_t y, size_t width, size_t height, size_t frameCount);

    void setAnimationSpeed(float value);
    void setAnimation(const std::string& name);
    void setCurrentFrame(size_t index);

private:
    sf::Sprite m_sprite;
    sf::Texture m_texture;
    std::unordered_map<std::string, std::vector<sf::IntRect>> m_frames;

    size_t m_currentIndex = 0;
    float m_animSpeed = 0.f;

    std::string m_animName;
    float m_elapsedTime = 0.f;
};
