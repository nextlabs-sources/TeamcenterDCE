#pragma once
#include <cstddef>
#include <iostream>
#include <locale>

class Section
{
public:
	std::string name;
	int len;

	Section(std::string &inName, int inLen);
	~Section();
};


