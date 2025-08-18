#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <random>
#include <algorithm>

#include "SafeTypes.hpp"
#include "Utils.h"
#include "Logger.h"

const sf::Vector2f DEFAULT_SIZE = { 800.f, 640.f };

class Camera 
{
public:
	Camera(const sf::Vector2f & size);

	void setCenter(const sf::Vector2f & center);
	void move(const sf::Vector2f & offset);
	void zoom(float factor);

	void setTarget(const sf::Vector2f * target);
	void setTarget(const sf::Vector2f & target);

	void setBounds(const sf::FloatRect & bounds);
	void setSize(const sf::Vector2f & value);
	void setSizeAndCenter(const sf::Vector2f & value);

	void setLerpSpeed(float speed) { m_lerpSpeed = speed; }
	float getLerpSpeed() const { return m_lerpSpeed; }

	sf::View getView() const { return m_view; }

	void shake(float duration, float intensity);
	void update(float deltaTime);
	void applyTo(sf::RenderTarget & target) const;
	void resetCamera();
	void resetCamera(const sf::Vector2f& size);

	bool isViewWithinBounds() const;

private:
	sf::View m_view;
	const sf::Vector2f* m_target = nullptr;
	std::optional<sf::FloatRect> m_bounds;

	float m_shakeTime = 0.f;
	float m_shakeDuration = 0.f;
	float m_shakeIntensity = 0.f;

	float m_lerpSpeed = 5.f;

	sf::Vector2f getShakenOffset() const;
};
