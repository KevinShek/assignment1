#include <iostream>
#include <fstream>
#include "ShapeStruct.h"
#include <vector>
#include <sstream>
#include "ReadConfig.h"
#include "UIStruct.h"
#include "ShapeProperties.h"

ShapeStruct AssigningPropertiesToShape(std::vector<std::string>& items, UIStruct& WindowSetting)
{
	ShapeStruct shape;
	ShapeProperties prop;
	shape.ShapeType = items[0];
	shape.Name = items[1];
	shape.TextSMFL = sf::Text(items[1], WindowSetting.myFont, WindowSetting.FontSize);
	shape.IntialPositionX = std::stof(items[2]);
	shape.IntialPositionY = std::stof(items[3]);
	shape.SpeedX = std::stof(items[4]);
	shape.SpeedY = std::stof(items[5]);
	std::vector<int> Colours;
	for (int i = 6; i < 9; ++i) {
		Colours.push_back(std::stoi(items[i]));
	}
	shape.Colour = prop.ConvertColourImGui(Colours);
	shape.size1 = std::stof(items[9]);
	if (items[0] == "Rectangle") {
		shape.size2 = std::stof(items[10]);
		shape.DrawableShape = sf::RectangleShape(sf::Vector2f(shape.size1, shape.size2));
		shape.DrawableShape2 = std::make_unique<sf::RectangleShape>(sf::Vector2f(shape.size1, shape.size2));
	}
	else if (items[0] == "Circle") {
		shape.DrawableShape = sf::CircleShape(shape.size1, shape.Segments);
		shape.DrawableShape2 = std::make_unique<sf::CircleShape>(shape.size1, shape.Segments);
	}
	prop.CreateShapeSFML2(shape);

	// Set Text initial Position
	shape.TextSMFL.setPosition(shape.IntialPositionX, shape.IntialPositionY);

	return shape;
}

void AssigningWindowToUI(std::vector<std::string>& items, UIStruct& window)
{
	window.WindowHeight = std::stoi(items[2]);
	window.WindowWidth = std::stoi(items[1]);
	//return window;
}

void AssigningFontToUI(std::vector<std::string>& items, UIStruct& window)
{
	window.FontPath = items[1];
	window.FontSize = std::stoi(items[2]);
	for (int i = 3; i < 6; ++i) {
		window.FontColour.push_back(std::stoi(items[i]));
	}
	if (!window.myFont.loadFromFile(window.FontPath))
	{
		// if we can't load the font, print an error to the error console and exit
		std::cerr << "Could not load font!\n";
		exit(-1);
	}
	//return window;
}

std::vector<ShapeStruct> ReadConfig::ReadingConfig(std::string& ConfigPath, UIStruct& window)
{
	std::ifstream fin(ConfigPath);
	std::vector<ShapeStruct> shapes;
	std::string item;
	std::string line;
	while (std::getline(fin, line))
	{
		//grabbing the line
		std::stringstream ss(line);
		std::vector<std::string> items;
		while (std::getline(ss, item, ' '))
		{
			items.push_back(item);
		}
		//std::cout << items[0] << std::endl;

		if (items[0] == "Circle" || items[0] == "Rectangle") {
			ShapeStruct shape = AssigningPropertiesToShape(items, window);
			shapes.push_back(std::move(shape));
		}
		else if (items[0] == "Window") {
			AssigningWindowToUI(items, window);
		}
		else if (items[0] == "Font") {
			AssigningFontToUI(items, window);
		}
	}
	return shapes;
}