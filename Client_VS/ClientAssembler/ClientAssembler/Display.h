#pragma once
#include <vector>


class Display {

public:
	int width, height;
	std::vector<std::vector<int>> colorGrid;


	Display(int x, int y);

	void DrawGrid();

};