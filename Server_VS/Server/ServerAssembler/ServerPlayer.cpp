#ifndef SERVER_PLAYER_H
#define SERVER_PLAYER_H

#include "ServerPlayer.h"

ServerPlayer::ServerPlayer()
{
	clientID = -1;
	clientSocket = INVALID_SOCKET;
}

ServerPlayer::ServerPlayer(int clientID, SOCKET clientSocket)
{
	this->clientID = clientID;
	this->clientSocket = clientSocket;
}

void ServerPlayer::ResetPlayer(int yourPosX, int yourPosY)
{
	curX = yourPosX;
	curY = yourPosY;
}

void ServerPlayer::DisconnectPlayer()
{
	//Close socket automatically for now
	int result = closesocket(clientSocket);
	if (result != 0) //if error occurred while trying to close socket
		std::cout << "Socket close failed - " << WSAGetLastError() << std::endl;
}

std::string ServerPlayer::GetSetupString() 
{
	std::string result = std::to_string(clientID) + "|" + std::to_string(curX) + '|' + std::to_string(curY);
	return result;
}

#endif