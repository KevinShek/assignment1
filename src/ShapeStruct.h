#pragma once
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <variant>

struct ShapeStruct
{
	std::string ShapeType;
	std::string Name;
	float SpeedX = 1.0f;
	float SpeedY = 0.5f;
	bool DrawShape = true;
	bool DrawText = true;
	int Segments = 32;
	float IntialPositionX = 10.0f;
	float IntialPositionY = 10.0f;
	std::vector<float> Colour = {0, 0, 0};
	float size1 = 0;
	float size2 = 0;
	std::variant<sf::CircleShape, sf::RectangleShape> DrawableShape;
	std::unique_ptr<sf::Shape> DrawableShape2;
	sf::Text TextSMFL;
};
