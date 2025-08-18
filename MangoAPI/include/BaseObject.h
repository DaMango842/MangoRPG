#pragma once

#include <SFML/Graphics.hpp>


class BaseObject
{
public:
	BaseObject() {};
	virtual ~BaseObject() {};

	virtual void update(float deltaTime) = 0;
	virtual void render(sf::RenderTarget& target) = 0;
};