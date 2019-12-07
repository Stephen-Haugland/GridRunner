#include "ClientPlayer.h"

ClientPlayer::ClientPlayer()
{
	id = -1;
	curX = -1;
	curY = -1;
}

ClientPlayer::ClientPlayer(int id, int spawnX, int spawnY)
{
	this->id = id;
	curX = spawnX;
	curY = spawnY;
}

bool ClientPlayer::PathContains(int curX, int curY)
{
	return playerPath.find(std::make_pair(curX, curY)) != playerPath.end();
}

void ClientPlayer::UpdatePlayerPos(int newX, int newY, bool isPath)
{
	curX = newX;
	curY = newY;
	if (isPath)
		playerPath.insert(std::make_pair(curX, curY));
}

