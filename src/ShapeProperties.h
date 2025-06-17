#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "ShapeStruct.h"

class ShapeProperties
{
public:
	ShapeProperties()
	{
	}
	~ShapeProperties()
	{
	}

	void CreateShapeSFML(ShapeStruct& shape);
	void CreateShapeSFML2(const ShapeStruct& shape);
	std::vector<float> ConvertColourImGui(std::vector<int>& Colour);

};