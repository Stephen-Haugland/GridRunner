#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

class ServerPlayer
{

public:
	int clientID = 0;
	SOCKET clientSocket = INVALID_SOCKET;
	int curX = -1;
	int curY = -1;

	//[CONSTRUCTOR]
	ServerPlayer(int clientID, SOCKET clientSocket);

	//[SETUP]
	void ResetPlayer(int yourPosX, int yourPosY);

	std::string GetSetupString();
};