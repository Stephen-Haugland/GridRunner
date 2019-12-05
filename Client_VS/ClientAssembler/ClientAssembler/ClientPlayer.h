#pragma once

#include <set>

class ClientPlayer
{
public:
	std::set<int> playerPath;
	int curX;
	int curY;

	ClientPlayer(int spawnX, int spawnY);


};