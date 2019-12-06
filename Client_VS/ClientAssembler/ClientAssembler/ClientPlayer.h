#pragma once

#include <set>

class ClientPlayer
{
public:
	//sets have O(log(n)) lookup, and is faster with smaller amounts of elements (which is what we expect the path to be)
	std::set<std::pair<int,int>> playerPath;
	int id;
	int curX;
	int curY;

	ClientPlayer(int id, int spawnX, int spawnY);

	bool PathContains(int curX, int curY);
	void ClientPlayer::UpdatePlayerPos(int newX, int newY, bool isPath);
};