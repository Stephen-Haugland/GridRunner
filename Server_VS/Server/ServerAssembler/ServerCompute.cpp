#ifndef SERVERCOMPUTE_H
#define SERVERCOMPUTE_H

#include "ServerCompute.h"

ServerCompute::ServerCompute(int x, int y)
{
	width = x;
	height = y;
}

void ServerCompute::MovePlayers()
{
	//Move Player To The Next Grid
	std::map<int, ServerPlayer>::iterator player;
	for (player = players.begin(); player != players.end(); player++)
	{
		if (player->second.state != 'U')
		{
			NextGridPosition(player->second.moveDirection, player->second.curX, player->second.curY, width, height);

			//Declare person dead if outside of grid (-1, -1 returned)
			if (player->second.curX == -1 || player->second.curY == -1)
				player->second.state = 'D';
			else //TODO: make proper move state based on wether current cell is in players captured territory
				player->second.state = 'M';
		}
	}

	//Check Conquer

	//Check Conflict
	std::map<int, ServerPlayer>::iterator player1;
	for (player1 = players.begin(); player1 != players.end(); player1++)
	{
		std::map<int, ServerPlayer>::iterator player2;
		for (player2 = players.begin(); player2 != players.end(); player2++)
		{
			if(player1->second.curX == player2->second.curX && player1->second.curY == player2->second.curY)
			{
				player1->second.state = 'D';
				player2->second.state = 'D';
			}
		}
	}
	// - check people on paths
	// - check people in each others positions

}

void ServerCompute::NextGridPosition(int playerDirection, 
									 int &curX, 
									 int &curY, 
									 int gridSizeX,
									 int gridSizeY)
{

	int tempX = curX;
	int tempY = curY;
	__asm
	{
		mov eax, playerDirection //stores direction in input register

		cmp eax, 0			//Compares if player direction is up
		jne notUp			//Checks next if not up		
		dec tempY			//If user presses W decrement Y position (Moves up 1 space)
		cmp tempY, 0			//Checks if out of range of grid
		jae endOfFunc
		mov tempX, -1		//If out of range, sets position to (-1,-1) (player dies)
		mov tempY, -1
		jmp endOfFunc

		notUp:				//Check if player moves down
			cmp eax, 1		//Compares if player direction is down
			jne notDown		//IF not going down move to next direction
			inc tempY		//IF going down increment y postion
			mov ebx, gridSizeY	
			cmp tempY, ebx	//Check if outside grid
			jb endOfFunc
			mov tempX, -1	//IF outside declare player dead
			mov tempY, -1
			jmp endOfFunc
		
		//... CHECK OTHER DIRECTIONS

		notDown:			//Check if player moves left
			cmp eax, 2
			jne notLeft
			dec tempX
			cmp tempX, 0
			jae endOfFunc
			mov tempX, -1
			mov tempY, -1
			jmp endOfFunc

		notLeft:			//Check if player moves right
			inc tempX
			mov ebx, gridSizeX
			cmp tempX, ebx
			jb endOfFunc
			mov tempX, -1
			mov tempY, -1

		endOfFunc:
	}
	curX = tempX;
	curY = tempY;
}

bool ServerCompute::CheckPathCompletion()
{
	return false;
}

bool ServerCompute::CheckConflict()
{
	return false;
}


//[PLAYER INITIAL SETUP]
void ServerCompute::SetPlayerSpawnPoint(int playerId)
{
	int curX, curY;
	do
	{
		curX = rand() % width;
		curY = rand() % height;

	} while (isConflictingSpawnPoint(curX, curY, playerId));
	players[playerId].ResetPlayer(curX, curY);
}

//Checks other player positions to see if current position will be on top of another player
bool ServerCompute::isConflictingSpawnPoint(int curX, int curY, int curID)
{
	std::map<int, ServerPlayer>::iterator player;
	for (player = players.begin(); player != players.end(); player++)
	{
		if (curID != player->first)	//no need to check the client thats about to spawn
		{
			if (player->second.curX == curX && player->second.curY == curY)
			{
				return true; // yes conflict (same space as another player)
			}
		}
	}

	return false; //no conflicts
}


//[PLAYER EXISTANCE FUNCTIONS]
bool ServerCompute::PlayerOnline(int id)
{
	return players.find(id) != players.end();
}

void ServerCompute::AddPlayer(int playerId, SOCKET playerSocket)
{
	players.insert({playerId, ServerPlayer(playerId, playerSocket)});
}

bool ServerCompute::RemovePlayer(int playerId)
{
	if (!PlayerOnline(playerId))
	{
		return false;
	}
	else
	{
		players[playerId].DisconnectPlayer();
		players.erase(playerId);
		return true;
	}
}

void ServerCompute::RemoveAllPlayers()
{
	std::map<int, ServerPlayer>::iterator player;
	for (player = players.begin(); player != players.end(); player++)
	{
		player->second.DisconnectPlayer();
	}
	players.clear();
}

#endif // !SERVERCOMPUTE_H
