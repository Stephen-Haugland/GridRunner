#pragma once
#include <vector>
#include <map>
#include <iostream>
#include "GridPoint.h"
#include "ClientPlayer.h"


//Libraries needed for both client and server
#include <winsock2.h>
#include <ws2tcpip.h>



class Display {

public:
	int ourId = -1;
	int width, height;
	//2D Vector for fast access to induvidual grid points and their state
	std::vector<std::vector<GridPoint>> colorGrid;
	//Lookup, deletion, insertion is O(log(n)) this is the preffered container if need to stor a low amount of elements 
	std::map<int, ClientPlayer> players = std::map<int, ClientPlayer>();

	//[CONSTRUCTORS]
	Display(int x, int y);
	 
	//[DRAWING]
	void DrawGrid();

	//[GRID UPDATE] (TODO: call draw grid after finishing update)
	void DestroyPlayerCells(int playerId);	//colors all belongings white, shows game over message (if current player)
	//void AddPlayerCells();				//converts all of path to be conquered
	void MovePlayer(int id, int nextX, int nextY, std::string state);		//adds to path (if needed), recolers
	void SetupNewPlayer(int id, int x, int y);

	//[PLAYER EXISTANCE UPDATES]
	bool PlayerOnline(int playerId);
	void AddPlayer(int playerId, int spawnX, int spawnY);
	bool RemovePlayer(int playerId);

	//[UTILITY]
	void SetCursorPosition(short CoordX, short CoordY);
	void SetDrawColor(bool r, bool g, bool b, bool isBright);
	void Clear();

	//[ACCESSORS/MUTATORS]
	void Display::SetOurID(int ourId);

};