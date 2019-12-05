#pragma once


class ServerCompute 
{
public:
	int width;
	int height;


	ServerCompute(int x, int y);

	void MovePlayers();

	//playerDirection: User will use WASD to move on console, WASD corresponds to an integer. W = 0, S = 1, A = 2, D = 3
	void NextGridPosition(int playerDirection, int &curX, int &curY, int gridSizeX, int gridSizeY);

	bool CheckPathCompletion();

	bool CheckConflict();

};