//Description: The client side of the assembler game (each user player this program)

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
#include <string>
#include <conio.h>
#include <Windows.h>

#include "Display.h" //Stores functions thjat alter game grid




//[FUNCTION PROTOTYPES]
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
bool isDead = false;

//Create a display class that handles all of grid updates
Display gameGrid(100, 60);

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

	//Ask for player initials
	//std::cout << "Please enter your initials (only 2 first characters will be sent to server): ";
	//std::string playerInitialsInput = "";
	//std::cin >> playerInitialsInput;
	//std::cin.clear();
	//std::cin.ignore(10000, '\n');
	//if(playerInitialsInput.length() )

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

	//TODO: Should really try the next address returned by getaddrinfo
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
			//Redraw grid only when apropriate messages were propccessed (only when ProcessMessage returns true)
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
			std:cout << "Server has closed the connection! Please open a new client!" << std::endl;

			stayInGame = false;
		}
		else
		{
			gameGrid.Clear();
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
			stayInGame = false;
		}
		PollDirectionInput();
	} while (stayInGame);

	//Shutdown client connection socket (no more messages will be sent)
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) 
	{
		std::cout << "shutdown failed: " << WSAGetLastError();
	}

	closesocket(ConnectSocket);
	WSACleanup();
	Sleep(10000); //Gives users time to read errors and quit messages

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

//[SERVERE MESSAGE FUNCTIONS]
//ChangePlayerDirection: Sends message about new;y selected direction imediatly
void ChangePlayerDirection(int newDirection)
{
	//Send new direction to server
	lastestDirection = newDirection;
	SendMsg("INPUT", std::to_string(newDirection), sendBuf);
}

//SendMsg: Sends a message to server
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
	//5 CHAR (function type) | 4 DIGIT INT (full character count) | ... (the rest of the message) ...

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
	vector<std::string> params;
	SplitParameters(params, msgContent);
	if (msgType == "PRINT")
	{
		std::cout << "Server lets you know - " << msgContent << std::endl;
		return false;
	}

	//Do not act upon any game messages from server except for setup if currently dead
	if (isDead)
	{
		return false;
	}

	if(msgType == "MOVEP")
	{
		if (params.size() % 4 == 0)
		{
			for (int i = 0; i < params.size(); i += 4)
			{
				if (gameGrid.MovePlayer(std::stoi(params[i]), std::stoi(params[i + 1]), std::stoi(params[i + 2]), params[i + 3]))
				{
					return true;	//Will update the entire grid
				}
				else
				{
					while (!gameGrid.needUpdates.empty())
					{
						gameGrid.DrawPoint(gameGrid.needUpdates.front().first, gameGrid.needUpdates.front().second);
						gameGrid.needUpdates.pop();
					}
				}
			}
		}
	}
	else if (msgType == "SETUP")
	{
		//FORMAT: id|x spawn of us|y spawn of us
		gameGrid.SetOurID(std::stoi(params[0]));
		gameGrid.AddPlayer(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]));
		return true;
	}
	else if (msgType == "NEWPL")
	{
		//FORMAT: id|x spawn of other|y spawn of other
		gameGrid.AddPlayer(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]));
		return true;
	}
	else if (msgType == "OLDPL")
	{
		//FORMAT: id|x spawn of other #1|y spawn of other #1|x spawn of other #2|y spawn of other #2
		if (params.size() % 3 == 0)
		{
			for (int i = 0; i < params.size(); i += 3)
			{
				gameGrid.AddPlayer(std::stoi(params[i]), std::stoi(params[i + 1]), std::stoi(params[i + 2]));
			}
			return true;
		}
		else
			return false;
	}
	else if(msgType == "DELPL")
	{
		//FORMAT: id to delete | restart avaliable
		int deleteID = std::stoi(params[0]);
		int restartAvalible = std::stoi(params[1]);
		if (deleteID == gameGrid.ourId)
		{
			isDead = true;

			gameGrid.Clear();
			std::cout << "Server has kicked you... You must've been a bad potato" << std::endl;

			return false;
		}
		else
		{
			gameGrid.RemovePlayer(deleteID);
			return true;
		}
	}
	else
	{
		std::cout << "Unknown message recieved - " << msgType << ": " << msgContent << std::endl;
	}


	return false; //if gotten to end do not redraw map
}

//Populates the parameter vector with delimited parameters from msg string (TODO: add delimiter parameter)
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