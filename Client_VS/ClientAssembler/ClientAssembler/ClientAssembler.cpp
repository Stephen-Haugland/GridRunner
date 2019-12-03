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

void Set_Cursor_Position(short CoordX, short CoordY)
//our function to set the cursor position.
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { CoordX,CoordY };

	SetConsoleCursorPosition(hStdout, position);
}


int main() 
{

	bool isDrawing = true;
	while (isDrawing)
	{
		//Testing if inline assembler works
		__asm
		{
			mov isDrawing, 0
		}
		if (isDrawing)
		{
			std::cout << "I'm drawing!" << std::endl;
		}
	}
	std::cout << "I'm DONE drawing!" << std::endl;

	Set_Cursor_Position(0, 4);


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

	#define DEFAULT_BUFLEN 512
	int recvbuflen = DEFAULT_BUFLEN;

	const char* sendbuf = "WHAT IS MY ID???";
	char recvbuf[DEFAULT_BUFLEN];

	int iResult;

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive data until the server closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);

	std::cout << "Final message recieved - " << recvbuf << std::endl;

	return 0;
}