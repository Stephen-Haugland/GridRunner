#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>

class ServerPlayer
{

public:
	int clientID = 0;
	SOCKET clientSocket;
	HANDLE clientThread;
	int curX = -1;
	int curY = -1;
	int moveDirection = 1;
	//M - move on own territory
	//P - move and add to path 
	//C - just conquered path (single frame)
	//D - dead (single frame)
	//U - dead and unconnected
	char state = 'M';	


	//[CONSTRUCTOR]
	ServerPlayer();
	ServerPlayer(int clientID, SOCKET clientSocket);
	void DisconnectPlayer();

	//[SETUP]
	void ResetPlayer(int yourPosX, int yourPosY);

	std::string GetSetupString();
	std::string GetFullPlayerState();
};