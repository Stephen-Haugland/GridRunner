#pragma once

#include <set>

class ClientPlayer
{
public:
	std::set<std::pair<int,int>> playerPath;
	int curX;
	int curY;

	ClientPlayer(int spawnX, int spawnY);

	bool PathContains(int curX, int curY);
	void ClientPlayer::UpdatePlayerPos(int newX, int newY, bool isPath);
};