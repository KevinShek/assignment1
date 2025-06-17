#include <string>
#include "ShapeProperties.h"
#include <vector>
#include "ShapeStruct.h"

void ShapeProperties::CreateShapeSFML(ShapeStruct& shape)
{
	if (auto* circle = std::get_if<sf::CircleShape>(&shape.DrawableShape)) {
		circle->setPointCount(shape.Segments);
		circle->setPosition(shape.IntialPositionX, shape.IntialPositionY);
		circle->setRadius(shape.size1);
		circle->setFillColor(sf::Color(sf::Uint8(shape.Colour[0]), sf::Uint8(shape.Colour[1]), sf::Uint8(shape.Colour[2])));
	}
	else if (auto* rect = std::get_if<sf::RectangleShape>(&shape.DrawableShape)) {
		rect->setPosition(shape.IntialPositionX, shape.IntialPositionY);
		rect->setSize(sf::Vector2f(shape.size1, shape.size2));
		rect->setFillColor(sf::Color(sf::Uint8(shape.Colour[0]), sf::Uint8(shape.Colour[1]), sf::Uint8(shape.Colour[2])));
	}
}

void ShapeProperties::CreateShapeSFML2(const ShapeStruct& shape)
{
	if (auto* circle = dynamic_cast<sf::CircleShape*>(shape.DrawableShape2.get())) {
		circle->setPointCount(shape.Segments);
		circle->setRadius(shape.size1);
	}
	else if (auto* rect = dynamic_cast<sf::RectangleShape*>(shape.DrawableShape2.get())) {
		rect->setSize(sf::Vector2f(shape.size1, shape.size2));
	}
	shape.DrawableShape2->setPosition(shape.IntialPositionX, shape.IntialPositionY);
	shape.DrawableShape2->setFillColor(sf::Color(sf::Uint8(shape.Colour[0]), sf::Uint8(shape.Colour[1]), sf::Uint8(shape.Colour[2])));
}

std::vector<float> ShapeProperties::ConvertColourImGui(std::vector<int>& Colours)
{
	std::vector<float> NewList;
	for (int i = 0; i < 3; i++)
	{
		float newC = Colours[i] / static_cast<float>(255);
		NewList.push_back(newC);
	}
	return NewList;
}