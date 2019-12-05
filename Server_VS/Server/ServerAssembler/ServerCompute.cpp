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

}

void ServerCompute::NextGridPosition(int playerDirection, int &curX, int &curY, int gridSizeX, int gridSizeY)
{
	__asm
	{
		mov eax, playerDirection 
		cmp eax, 0					//Compares the playerDirection, check if move is up
		jne notUp					
		dec curY					//If user presses W decrement Y (Moves up 1 space)
		cmp curY, 0					//Checks if out of range of grid
		ja notAbove
		mov curX, -1				//If out of range, sets position to (-1,-1) player dies
		mov curY, -1

		notAbove:
		notUp:						//Check if player moves down
			cmp eax, 1
			jne notDown
			dec curY
			mov ebx, gridSizeY
			cmp curY, ebx
			jb notBelow
			mov curX, -1
			mov curY, -1
		
		notBelow:
		notDown:					//Check if player moves left
			cmp eax, 2
			jne notLeft
			dec curX
			cmp curX, 0
			ja notOutL
			mov curX, -1
			mov curY, -1

		notOutL:
		notLeft:					//Check if player moves right
			inc curX
			mov ebx, gridSizeX
			cmp curX, ebx
			jb notOutR
			mov curX, -1
			mov curY, -1

		notOutR:
	
	}
}

bool ServerCompute::CheckPathCompletion()
{
	
}

bool ServerCompute::CheckConflict()
{

}



#endif // !SERVERCOMPUTE_H
