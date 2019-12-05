//Description: The client side of the assembler game (each user player this program)

//References:
//C++ Networking - https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock
//Main return values - https://stackoverflow.com/questions/204476/what-should-main-return-in-c-and-c


//Libraries needed for both client and server
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

//Libraries used for debug of server
#include <iostream>
#include <string>

#include <conio.h>
#include <Windows.h>

//Our own classes
#include "Display.h"


//FUNCTION PROTOTYPES
// - Movement Input 
// - (0 is W (Up), 1 is S (Down), 2 is A (left), 3 is D (Left))
int lastestDirection = 1;
bool wPressed = true;
bool sPressed = true;
bool aPressed = true;
bool dPressed = true;
void PollDirectionInput();
void ChangePlayerDirection(int newDirection);
// - Networking
#define DEFAULT_BUFLEN 512
char sendBuf[DEFAULT_BUFLEN];
int iSendResult;
bool SendMsg(std::string messageType, std::string messageContent, char(&sendBuf)[512]);
bool ProcessMessage(char(&recvBuf)[512]);
void SplitParameters(vector<std::string>& parameters, std::string content);

//Initialize client socket object
SOCKET ConnectSocket = INVALID_SOCKET;

//Create a display class that handles all of grid updates
Display gameGrid(50, 30);

int main() 
{

	//[INITIALIZING WINSOCK]
	//Creating the networking data object to use Winsock dlls
	WSADATA wsaData;

	//Initializing winsock
	int startupResult;
	startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);    //using 2.2 version of winsock
	//Checking for errors
	if (startupResult != 0)
	{
		std::cout << "WSAS failed to start because - " << startupResult << std::endl;
		return 1; //to indicate non normal exit
	}

	std::cout << "Success - Winsock object initialized!" << std::endl;



	//[CREATING CLIENT SOCKET]
	//Create object that contains the socket adress and type information
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//Ask for server ip address (required)
	std::cout << "Please enter a valid server address: ";
	std::string serverIPinput = "";
	std::cin >> serverIPinput;
	std::cin.clear();
	std::cin.ignore(10000, '\n');
	PCSTR serverIp = serverIPinput.c_str();

	#define DEFAULT_PORT "27015"
	// Resolve the server address and port
	int addressResult = getaddrinfo(serverIp, DEFAULT_PORT, &hints, &result);
	if (addressResult != 0) 
	{
		std::cout << "Get address info failed - " << addressResult << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "Success - Address Resolved!" << std::endl;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server (and check if valid)
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) 
	{
		std::cout << "Opening client socket failed - " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	std::cout << "Success - client socket created!" << std::endl;

	// [CONNECT TO SERVER]
	int connectResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (connectResult == SOCKET_ERROR) 
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	//TODO: IMPLEMENT BELOW
	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message
	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) 
	{
		std::cout << "Connecting to server has failed - " << connectResult << std::endl;
		WSACleanup();
		return 1;
	}

	std::cout << "Success - client connected to server!" << std::endl;

	//[ENTER GAME LOOP]
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	SendMsg("ACEPT", "1", sendBuf);

	//Make socket non blocking for proper execution of game loop (even when no messages are being recieved)
	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(ConnectSocket, FIONBIO, &mode);

	// Receive data until the server closes the connection (the game loop pretty much)
	bool stayInGame = true;
	int iResult;
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			//Redraw grid only when apropriate messages were propccessed
			if (ProcessMessage(recvbuf))
				gameGrid.DrawGrid();
		}
		else if (iResult == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			//Connection would block (no messages, instead continue with game loop)
		}
		else if (iResult == 0)
		{
			gameGrid.Clear();
			printf("Connection closed\n");
			stayInGame = false;
		}
		else
		{
			gameGrid.Clear();
			printf("recv failed: %d\n", WSAGetLastError());
			stayInGame = false;
		}
		PollDirectionInput();
	} while (stayInGame);

	// Shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
	}

	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}

//Is run every game loop iteration to check for new user input (only the most recent key pressed is sent to server)
void PollDirectionInput()
{
	// - (0 is W (Up), 1 is S (Down), 2 is A (left), 3 is D (Left))
	if (GetAsyncKeyState(0x57) && lastestDirection != 0 && !wPressed) //If pressing up (W) and its a new direction
	{
		ChangePlayerDirection(0);
		wPressed = true;
	}
	else if (GetAsyncKeyState(0x53) && lastestDirection != 1 && !sPressed) //If pressing down (S) and its a new direction
	{
		ChangePlayerDirection(1);
		sPressed = true;
	}
	else if (GetAsyncKeyState(0x41) && lastestDirection != 2 && !aPressed) //If pressing left (A) and its a new direction
	{
		ChangePlayerDirection(2);
		aPressed = true;
	}
	else if (GetAsyncKeyState(0x44) && lastestDirection != 3 && !dPressed) //If pressing right (D) and its a new direction
	{
		ChangePlayerDirection(3);
		dPressed = true;
	}

	//Disable keys not pressed
	if(!GetAsyncKeyState(0x57))
	{
		wPressed = false;
	}
	if (!GetAsyncKeyState(0x53))
	{
		sPressed = false;
	}
	if (!GetAsyncKeyState(0x41))
	{
		aPressed = false;
	}
	if (!GetAsyncKeyState(0x44))
	{
		dPressed = false;
	}
}

//[Server Message Functions]
void ChangePlayerDirection(int newDirection)
{
	//UI Updates?

	//Send new direction to server
	lastestDirection = newDirection;
	SendMsg("INPUT", std::to_string(newDirection), sendBuf);
}


//Sends a message to server
//TRUE if completed succesfully, FALSE if failed

bool SendMsg(std::string messageType, std::string messageContent, char(&sendBuf)[512])
{
	//Check basic contraints
	if (ConnectSocket == INVALID_SOCKET || messageType == "" || messageContent == "" || messageType.length() != 5)
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
	std::cout << "Message Packet Complete: " << finalMsg << std::endl;

	for (unsigned i = 0; i < finalMsg.length(); i++)
	{
		sendBuf[i] = finalMsg[i];
	}

	int iResult = send(ConnectSocket, sendBuf, 512, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

//TRUE when a message that prompts to redraw the grid was proccessed, FALSE if error or utility message
bool ProcessMessage(char(&recvBuf)[512])
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

	//LIST OF POSSIBLE INCOMING MESSAGES (from server)
	// - PRINT: Server wants us to print a message onto the screen (TODO: clears screen)

	//Proper Action based on message type
	if (msgType == "PRINT")
	{
		std::cout << "Server lets you know - " << msgContent << std::endl;
		return false;
	}
	else if (msgType == "SETUP")
	{
		//FORMAT: id|x spawn|y spawn
		vector<std::string> params;
		SplitParameters(params, msgContent);
		gameGrid.SetupNewPlayer(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]));
		return true;
	}
	else
	{
		std::cout << "Unknown message recieved - " << msgType << ": " << msgContent << std::endl;
	}


	return false; //if gotten to end do not redraw map
}

//populates the parameter vector with delimited parameters from msg string (TODO: add delimiter parameter)
void SplitParameters(vector<std::string> &parameters, std::string content)
{
	std::string curParameter = "";
	for (int i = 0; i < content.length(); i++)
	{
		if (content[i] == '|')
		{
			if (content != "")
			{
				parameters.push_back(curParameter);
				curParameter = "";
			}
		}
		else
			curParameter += content[i];
	}

	if (curParameter != "")
	{
		parameters.push_back(curParameter);
	}
}