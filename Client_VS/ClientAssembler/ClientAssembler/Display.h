#pragma once
#include <vector>
#include <iostream>


//Libraries needed for both client and server
#include <winsock2.h>
#include <ws2tcpip.h>




class Display {

public:
	int width, height;
	std::vector<std::vector<int>> colorGrid;

	//[CONSTRUCTORS]
	Display(int x, int y);

	//[DRAWING]
	void DrawGrid();

	//[GRID UPDATE] (TODO: call draw grid after finishing update)
	void DestroyPlayerCells();	//colors all belongings white, shows game over message
	void AddPlayerCells();		//converts all of path to be conquered
	void MovePlayer();			//adds to path (if needed), recolers

	//[COMPUTE]
	std::pair<int,int> NextGridPosition(int playerId);
	void MovePlayer();	//Simply changes current player location on grid, saves last position?
	void PerformNext(); //Determines outcome for each player based on their position -> death, conquered cells, move in territory, move and ad


	//[UTILITY]
	void SetCursorPosition(short CoordX, short CoordY);
	void SetDrawColor(bool r, bool g, bool b, bool isBright);


};