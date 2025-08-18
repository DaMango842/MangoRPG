#pragma once

#include <SFML/Graphics.hpp>

enum class Direction { Down, Left, Right, Up };


class Animation
{
public:
	Animation() = default;
	virtual ~Animation() = default;

	virtual void update(float deltaTime) = 0;
	virtual void render(sf::RenderTarget& target) = 0;

};