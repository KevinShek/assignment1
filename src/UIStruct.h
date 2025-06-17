#pragma once
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>

struct UIStruct
{
	int WindowWidth = 1280;
	int WindowHeight = 720;
	std::string FontPath = "fonts\\SacrificeDemo.ttf";
    sf::Font myFont;
	int FontSize = 18;
	std::vector<int> FontColour;
};