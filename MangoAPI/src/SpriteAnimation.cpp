#include "SpriteAnimation.h"

bool SpriteAnimation::loadTexture(const std::string& path)
{
	if (!m_texture.loadFromFile(path))
		return false;

	m_sprite.setTexture(m_texture);
	return true;
}

void SpriteAnimation::setPosition(const sf::Vector2f& pos)
{
	m_sprite.setPosition(pos);
}

void SpriteAnimation::setPosition(float x, float y)
{
	m_sprite.setPosition(x, y);
}


void SpriteAnimation::update(float deltaTime)
{
	if (m_frames.empty() || m_animName.empty() || !m_frames.contains(m_animName) || m_animSpeed <= 0.f)
		return;

	m_elapsedTime += deltaTime;

	const auto& frames = m_frames[m_animName];
	float frameDuration = 1.f / m_animSpeed;

	if (m_elapsedTime >= frameDuration) {
		m_elapsedTime -= frameDuration;
		m_currentIndex = (m_currentIndex + 1) % frames.size();
		m_sprite.setTextureRect(frames[m_currentIndex]);
	}
}


void SpriteAnimation::render(sf::RenderTarget& target)
{
	target.draw(m_sprite);
}

const std::vector<sf::IntRect>& SpriteAnimation::createAnimation(const std::string& anim_name, size_t x, size_t y, size_t width, size_t height, size_t frameCount)
{
	auto& frames = m_frames[anim_name];
	frames.clear();

	for (size_t i = 0; i < frameCount; i++) {
		frames.emplace_back(
			static_cast<int>((x + i) * width), 
			static_cast<int>(y * height),
			static_cast<int>(width),
			static_cast<int>(height)
		);
	}
	return frames;
}

void SpriteAnimation::setAnimationSpeed(float value)
{
	m_animSpeed = value;
}

void SpriteAnimation::setAnimation(const std::string& name)
{
	if (m_animName != name && m_frames.contains(name)) {
		m_animName = name;
		m_currentIndex = 0;
		m_elapsedTime = 0.f;
		m_sprite.setTextureRect(m_frames[m_animName][0]);
	}
}

void SpriteAnimation::setCurrentFrame(size_t index)
{
	if (m_frames.contains(m_animName)) {
		const auto& frames = m_frames[m_animName];
		if (index < frames.size()) {
			m_currentIndex = index;
			m_sprite.setTextureRect(frames[index]);
		}
	}
}

