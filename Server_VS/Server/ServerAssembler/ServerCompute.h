#pragma once

#include <map>
#include "ServerPlayer.h"

class ServerCompute 
{
public:
	int width;
	int height;

	//The players data structure stores information pertaining to induvidual clients connected to the server 
	//Lookup, deletion, insertion is O(log(n)) this is the preffered container if need to stor a low amount of elements 
	std::map<int, ServerPlayer> players = std::map<int, ServerPlayer>();

	//[CONSTRUCTOR]
	ServerCompute(int x, int y);

	//[PLAYER STATE UPDATES]
	//Notes:
	// - playerDirection: User will use WASD to move on console
	//   WASD corresponds to an integer. W = 0, S = 1, A = 2, D = 3
	void MovePlayers();
	void NextGridPosition(int playerDirection, int &curX, int &curY, int gridSizeX, int gridSizeY);
	bool CheckPathCompletion();
	bool CheckConflict();

	//[PLAYER INITIAL SETUP]
	void SetPlayerSpawnPoint(int playerId);
	bool isConflictingSpawnPoint(int curX, int curY, int curID);

	//[PLAYER EXISTANCE UPDATES]
	bool PlayerOnline(int playerId);
	void AddPlayer(int playerId, SOCKET playerSocket);
	bool RemovePlayer(int playerId);
	void RemoveAllPlayers();
};
