#pragma once
#include <vector>
#include "ShapeStruct.h"
#include <iostream>
#include "UIStruct.h"

class ReadConfig
{
public:
	ReadConfig(){}
	std::vector<ShapeStruct> ReadingConfig(std::string& Text, UIStruct& window);
};