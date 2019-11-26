//Description: The client side of the assembler game (each user player this program)

//References:
//C++ Networking - https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock
//Main return values - https://stackoverflow.com/questions/204476/what-should-main-return-in-c-and-c
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

//[FUNCTION PROTOTYPES]
void ShowGameMessage(std::string msg, int type);
void ProccessServerMessage(char msgBuffer[512]);
void CheckQuitKey();

bool closeGame = false;

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

	//Initialize client socket object
	SOCKET ConnectSocket = INVALID_SOCKET;

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
	const char* sendbuf = "WHAT IS MY ID???";

	#define DEFAULT_BUFLEN 512
	int recvbuflen = DEFAULT_BUFLEN;

	char recvbuf[DEFAULT_BUFLEN];

	int iResult;

	// Send an initial buffer
	/*iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);*/

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive data until the server shuts down (todo)
	do {

		//Check for incoming messages
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			//printf("Bytes received: %d\n", iResult);
			ProccessServerMessage(recvbuf);
		}
		else if (iResult == 0)
		{
			//no bytes recieved
		}
		else
		{
			std::cout << "Recieving message has failed becasue " << WSAGetLastError() << std::endl;
			break;
		}

		CheckQuitKey();
	} while (!closeGame);

	std::cout << "Game Shutting down!" << std::endl;

	closesocket(ConnectSocket);
	WSACleanup();

	std::cout << "Game Finished Exiting Networking!" << std::endl;

	return 0;
}

void ProccessServerMessage(char msgBuffer[512])
{
	//MESSAGES FORMAT:
	//CHAR(5) - message type 
	//CHAR(4) - message size
	//VARCHAR(501) -message content
	//eg. PRINT|0014|Hi!

	//Get the start of message to match to message type
	//Incoming message list:
	// - INITP - Initialize the current player (us) (includes id and spawn point)
	// - PLMOV - A player move update
	// - PLADD - A new player has joined
	// - PLSUB - An old player has left
	// - PLOLD - All previous players (id's and positions)
	// - PRINT - Print the message array to the console



	//Get the identifying information of the message (which type)
	std::string header = "";
	for (int i = 0; i < 5; i++)
		header += msgBuffer[i];

	//Get message length
	std::string lengthStr = "";
	int msgLength = -1;
	bool validNumber = true;
	for (int g = 6; g < 10; g++)
	{
		lengthStr += msgBuffer[g];
		if (!std::isdigit(msgBuffer[g]))
		{
			validNumber = false;
		}
	}
	if (!validNumber)
	{
		std::cout << "Failed reading invalid sized message!" << std::endl;
		return;
	}
	msgLength = std::stoi(lengthStr);


	//Get message content
	std::string message = "";
	for (int h = 0; h < msgLength; h++)
		message += msgBuffer[h];
	
	//Check if message is long enough to accept
	if (msgLength < 11 || msgLength > 512)
		return;

	//Properly call the correct function
	if (header == "PRINT")
	{
		ShowGameMessage("Server Message Recieved - " + message, 1);
	}
	else
	{
		ShowGameMessage("Server Message Type Could not accepted - " + header, 3);
	}
}


//void PerformMove();	//Send from server? or client side?
//void RedrawGrid();
//void 


//UTILITY
void ShowGameMessage(std::string msg, int type)
{
	//Add colors of messages based on severety 1 - 3

	//TODO: clear Cmd + do in assembler + in red?
	std::cout << msg << std::endl;
}

//Returns true if q key has been pressed (to quit server)
void CheckQuitKey()
{
	if (GetAsyncKeyState(0x10) && GetAsyncKeyState(0x51)) //IF Q key down (upper case)
	{
		closeGame = true;
	}
}