#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

#include "ServerPlayer.h"

ServerPlayer::ServerPlayer(int clientID, SOCKET clientSocket)
{
	this->clientID = clientID;
	this->clientSocket = clientSocket;
}

void ServerPlayer::ResetPlayer(int yourPosX, int yourPosY)
{
	//Generate new spawn point until not interfering with anyone else
	curX = yourPosX;
	curY = yourPosY;
}

std::string ServerPlayer::GetSetupString() 
{
	std::string result = std::to_string(clientID) + "|" + std::to_string(curX) + '|' + std::to_string(curY);
	return result;
}

#endif