//Description: The server side of the assembler game (only one computer runs this program)

//References (used in parts of project):
//C++ Networking (about half of the code) - https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock
//Main return values meanings - https://stackoverflow.com/questions/204476/what-should-main-return-in-c-and-c
//How to accept multiple connections (threading) - https://stackoverflow.com/questions/15185380/tcp-winsock-accept-multiple-connections-clients/15185627
//Non blocking accept example - https://github.com/pauldotknopf/WindowsSDK7-Samples/tree/3f2438b15c59fdc104c13e2cf6cf46c1b16cf281/netds/winsock/accept
//WSADPOLL example - https://www.youtube.com/watch?v=--DXAedVWks
//Show Server IP - https://github.com/angrave/SystemProgramming/wiki/Networking,-Part-2:-Using-getaddrinfo
//Thread access to data - https://stackoverflow.com/questions/6171350/how-to-pass-data-to-running-thread
//Polling keystates (windows) - https://www.youtube.com/watch?v=VuhE8wuYKEE
//Storing size of message in header - https://stackoverflow.com/questions/35732112/c-tcp-recv-unknown-buffer-size


//Libraries needed for both client and server
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

//Libraries used for debug of server
#include <iostream>
#include <ctime>
#include <chrono>
#include <process.h>
#include <vector>
#include "ServerCompute.h"


//[GLOBAL CONSTANTS]
//Game Settings
#define DEFAULT_BUFLEN 512	//messages limited to 512 charecters (can make it to be more)

//[PROTOTYPE FUNCTIONS]
//Networking
// - setup
void AcceptConnections(SOCKET& listenSocket);
unsigned __stdcall StartClientThread(void* data);
std::string AllPositionsString(int ignoreID);
void CheckQuitKey();
// - send Functions
void SendOne(SOCKET socket, std::string messageType, std::string messageContent, char(&sendBuf)[512]);
void SendAll(std::string messageType, std::string messageContent, char(&sendBuf)[512]);
void SendAllExcept(std::string messageType, std::string messageContent, int ignoreID, char(&sendBuf)[512]);
void SendPlayerStates(char(&sendBuf)[512]);
bool WriteToSendBuffer(std::string messageType, std::string messageContent, char(&sendBuf)[512]);
// - receive Functions
bool ProcessMessage(char(&recvBuf)[512], int clientID);

//[GLOBAL STORAGE VARIABLES]
ServerCompute compute(100, 60);
int curClients = 0;	//client counter (ensures all clients have unique ids)
char sendBufS[DEFAULT_BUFLEN];

//[THREAD CONTROL VARIABLES]
bool closeAllThreads = false;

int main() {

	//[CREATING THE NETWORKING OBJECT]
	WSADATA wsaData;

	//Initializing winsock
	int startupResult;
	startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);    //using 2.2 version of winsock
	//Checking for errors
	if (startupResult != 0)
	{
		std::cout << "WSAS failed to start because - " << startupResult << std::endl;
		return 1; //to indicate an non normal exit
	}

	std::cout << "Success - setup networking object!" << std::endl;


	//[INITIALIZING OF THE SOCKET OBJECT]
	//state the port to which clients should connect
#define DEFAULT_PORT "27015"

//get the final address to which clints connect
	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int addressResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (addressResult != 0)
	{
		std::cout << "Getting the address failed - " << addressResult << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "Success - got the full address of the port!" << std::endl;



	//[SHOW THE SERVER IP]
	struct addrinfo* p;
	char host[256];
	for (p = result; p != NULL; p = p->ai_next)
	{
		getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
		std::cout << std::endl << "Connect to this IP address: " << host << std::endl;
	}
	std::cout << std::endl;



	//[CREATE THE LISTENING SOCKET]
	SOCKET listenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "Creating socket failed - " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	std::cout << "Success - initialized the socket object!" << std::endl;

	//[MARK SOCKET FOR REUSE]
	int reuse = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
	{
		std::cout << "Creating socket failed - " << WSAGetLastError() << std::endl;
	}
	std::cout << "Success - Will be able to reuse socket!" << std::endl;


	//[BIND TO THE SOCKET]
	//Server must be bound to socket to accept client connections
	int socketBind = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (socketBind == SOCKET_ERROR)
	{
		std::cout << "Bind failed - " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result); //Adress information no longer needed

	std::cout << "Success -  bound server the socket object!" << std::endl;


	//[LISTEN ON THE SOCKET]
	//Server must listen to the chosen port to accept incoming connection requests
	//Below Sets the socket to listen to and the max connection allowed
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "Listen failed - " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Success -  set the socket to listen to on server!" << std::endl;

	//[CONTINUOUSLY ACCEPT INCOMING CONNECTIONS]
	//A loop constantly checks for any clients that want to join and creates a seperate thread for each to be proccessed
	//This continues until user presses Q on the key board
	//Movement timing reference: https://stackoverflow.com/questions/2808398/easily-measure-elapsed-time
	std::chrono::steady_clock::time_point lastMoveUpdateS = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point lastMoveUpdateE;

	while (!closeAllThreads)
	{
		CheckQuitKey();
		AcceptConnections(listenSocket);
		lastMoveUpdateE = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(lastMoveUpdateE - lastMoveUpdateS).count() > 250)
		{
			compute.MovePlayers();
			lastMoveUpdateS = std::chrono::steady_clock::now();
			SendPlayerStates(sendBufS);
		}
	}


	//Wait 5 sec for threads to finish execution
	closeAllThreads = true;
	std::cout << "SERVER CLEANING FOR 5 SECONDS!" << std::endl;
	time_t startCountdown = time(0);
	while (time(0) - startCountdown < 5) {}

	//Close remaining connections (should't be any)
	compute.RemoveAllPlayers();

	std::cout << "Success - server is closing down!" << std::endl;
	closesocket(listenSocket);
	WSACleanup();
	std::cout << "Success - server is shut down!" << std::endl;

	return 0;
}

//Constantly polls the listening socket for any incoming connection requests and opens up client sockets that will be handled on a seperate thread
void AcceptConnections(SOCKET& listenSocket)
{
	WSAPOLLFD listeningSocketFD = {};
	listeningSocketFD.fd = listenSocket;
	listeningSocketFD.events = POLLRDNORM;	//only checking if can read w/o blocking (for incoming requests)
	listeningSocketFD.revents = POLLRDNORM;

	if (WSAPoll(&listeningSocketFD, 1, 1) > 0) //waits 1 ms until checks for new connections
	{
		//if a connection was requested
		if (listeningSocketFD.revents & POLLRDNORM)
		{
			// Accept a client socket
			compute.AddPlayer(curClients, accept(listenSocket, NULL, NULL));
			//See results
			if (compute.players[curClients].clientSocket == INVALID_SOCKET)
			{
				std::cout << "Accept connection failed - " << WSAGetLastError() << std::endl;
			}
			else
			{
				std::cout << "Success - accepted a connection!" << std::endl;
				unsigned threadID;
				compute.players[curClients].clientThread = (HANDLE)_beginthreadex(NULL, 0, &StartClientThread, (void*)curClients, 0, &threadID);
				curClients++;
			}
		}
	}
}

//Handles induvidual client recieving of messages
unsigned __stdcall StartClientThread(void* data)
{
	//Get the random number generated seeded for player spawn points
	srand(time(NULL));

	char recvbuf[DEFAULT_BUFLEN];
	char sendBuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	//Setup player object in compute class
	int id = (int) data;
	compute.SetPlayerSpawnPoint(id);

	//[SEND INTITIAL CLIENT DATA]
	//Send the spawn point of new player to everyone
	SendAllExcept("NEWPL", compute.players[id].GetSetupString(), id, sendBuf);
	//Send the spawn info to new player (defferent function for different setup)
	SendOne(compute.players[id].clientSocket, "SETUP", compute.players[id].GetSetupString(), sendBuf);

	//Send current positions of other players to new player
	std::string allPositions = AllPositionsString(compute.players[id].clientID);
	if(allPositions != "")
		SendOne(compute.players[id].clientSocket, "OLDPL", allPositions, sendBuf);


	//Recieve messages until user shuts of
	do
	{
		iResult = recv(compute.players[id].clientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			std::cout << "Success - Recieved message from client #" << compute.players[id].clientID << std::endl;
			std::cout << "Message is - " << recvbuf << std::endl;
			ProcessMessage(recvbuf, id);
		}
		else if (iResult == 0 || (iResult == -1 && WSAGetLastError() == WSAECONNRESET))
		{
			std::cout << "Client disconnected " << std::endl;
			SendAllExcept("DELPL", std::to_string(id) + "|0", id, sendBuf);
			break;
		}
		else
		{
			if (closeAllThreads)
				return 0; //server was shut down already
			std::cout << "Recieving client #" << compute.players[id].clientID << " message has failed becasue " << WSAGetLastError() << std::endl;
			break;
		}
	} while (!closeAllThreads);

	compute.RemovePlayer(id);
	return 0;
}




//Make sure to clear end of sendBuf to get rid of other client's data
void SendOne(SOCKET socket, std::string messageType, std::string messageContent, char (&sendBuf)[512])
{
	//Write message to the buffer if not done already
	if (messageContent != "" && messageType != "")
	{
		if (!WriteToSendBuffer(messageType, messageContent, sendBuf))
			return;
	}

	if (messageType == "SETUP")
	{
		std::cout << messageType << " - " << messageContent << std::endl;
	}

	// Send over the message to sender
	int iSendResult = send(socket, sendBuf, 512, 0);
	if (iSendResult == SOCKET_ERROR)
	{
		std::cout << "Sending message to client has failed because " << WSAGetLastError() << std::endl;
		closesocket(socket);
		socket = INVALID_SOCKET;
		return;
	}
}

void SendAll(std::string messageType, std::string messageContent, char(&sendBuf)[512])
{

	if (!WriteToSendBuffer(messageType, messageContent, sendBuf))
		return;

	std::map<int, ServerPlayer>::iterator player;
	for (player = compute.players.begin(); player != compute.players.end(); player++)
	{
		if (player->second.clientSocket != INVALID_SOCKET)
		{
			SendOne(player->second.clientSocket, messageType, messageContent, sendBuf);
		}
	}

	std::cout << "Success - Sent message top all: " << messageType << " - " << messageContent << std::endl;
}

void SendAllExcept(std::string messageType, std::string messageContent, int ignoreID, char(&sendBuf)[512])
{
	if (!WriteToSendBuffer(messageType, messageContent, sendBuf) || ignoreID < 0)
		return;

	std::map<int, ServerPlayer>::iterator player;
	for (player = compute.players.begin(); player != compute.players.end(); player++)
	{
		if (player->second.clientSocket != INVALID_SOCKET && ignoreID != player->first)
		{
			SendOne(player->second.clientSocket, "", "", sendBuf);
		}
	}

	std::cout << "Success - Sent message to all but " << std::to_string(ignoreID) << ": " << messageType << " - " << messageContent << std::endl;
}

void SendPlayerStates(char(&sendBuf)[512])
{
	std::string finalStates = "";
	std::map<int, ServerPlayer>::iterator player;
	for (player = compute.players.begin(); player != compute.players.end(); player++)
	{
		if (player->second.state != 'U')
		{
			finalStates += player->second.GetFullPlayerState();
		}
	}

	if (finalStates != "")
	{
		finalStates.pop_back();
		SendAll("MOVEP", finalStates, sendBuf);
	}

	//LARGE TODO: HANDLE MESSAGES larger than limit of 512 charecters
}

std::string AllPositionsString(int ignoreID)
{
	std::string result = "";
	std::map<int, ServerPlayer>::iterator player;
	for (player = compute.players.begin(); player != compute.players.end(); player++)
	{
		if (player->second.clientSocket != INVALID_SOCKET && player->first != ignoreID)
		{
			result += player->second.GetSetupString() + "|";
		}
	}
	if(result != "")
		result.pop_back();

	//LARGE TODO: handle output if string to large!!! more characters than message buffer allows

	return result;
}

//Returns true if q key has been pressed (to quit server)
void CheckQuitKey()
{
	if (GetAsyncKeyState(0x10) && GetAsyncKeyState(0x51)) //IF Q key down (upper case)
	{
		closeAllThreads = true;
	}
}

//Writes the header along with final length of message to the buffer
//TRUE if completed succesfully, FALSE if failed
bool WriteToSendBuffer(std::string messageType, std::string messageContent, char(&sendBuf)[512])
{
	//Check basic contraints
	if (messageType == "" || messageContent == "" || messageType.length() != 5)
	{
		return false;
	}

	float finalMsgLength = (messageType.length() + messageContent.length() + 6);
	std::string finalMsgLengthStr = "";
	for (int d = 1000; d >= 10; d /= 10)
	{
		if (finalMsgLength < d)
			finalMsgLengthStr += "0";
	}
	finalMsgLengthStr += std::to_string((int)finalMsgLength);

	std::string finalMsg = messageType + "|" + finalMsgLengthStr + "|" + messageContent;

	for (unsigned i = 0; i < finalMsg.length(); i++)
	{
		sendBuf[i] = finalMsg[i];
	}

	return true;
}

bool ProcessMessage(char(&recvBuf)[512], int clientID)
{
	//MESSAGE FORMAT:
	//5 CHAR (function type) | 4 DIGIT INT (full msg length) | ... (the rest of the message) ...

	//Get the header info
	std::string msgType = "";
	for (int i = 0; i < 5; i++)
	{
		msgType += recvBuf[i];
	}

	//Get the message length info
	std::string msgLengthStr = "";
	int msgLength = 0;
	for (int i = 6; i < 10; i++)
	{
		if (isdigit(recvBuf[i]))
			msgLengthStr += recvBuf[i];
		else
			return false;
	}
	msgLength = std::stoi(msgLengthStr);

	//Get the full message contents
	std::string msgContent = "";
	for (int i = 11; i < msgLength; i++)
	{
		msgContent += recvBuf[i];
	}

	//LIST OF POSSIBLE INCOMING MESSAGES (from clients)
	// - ACEPT: validates that server has accepted the connectiopn (first message)
	// - INPUT: new user direction selected

	//Proper Action based on message type
	if (msgType == "ACEPT")
	{
		std::cout << "Client #" << clientID << "accepted on to the server!" << msgContent << std::endl;
	}
	else if (msgType == "INPUT")
	{
		std::cout << "Client #" << clientID <<"has changed their direction to - " << msgContent << std::endl;
		compute.players[clientID].moveDirection = std::stoi(msgContent);
	}
	else
	{
		std::cout << "Unknown message recieved - " << msgType << ": " << msgContent << " from " << clientID << std::endl;
	}


	return true;
}