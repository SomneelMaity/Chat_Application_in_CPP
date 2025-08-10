#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>

using namespace std;

// code needs to use windows socket library
#pragma comment(lib,"ws2_32.lib")

/*
	// Initialize winsock library
	// create the socket
	// get ip and port
	// bind ip/port with the socket
	// listen on the socket
	// accept
	// receive and send
	// close the socket
	// cleanup the winsock
*/

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET> &clients)
{
	// send/recv
	cout << "Client Connected\n";
	// Whatever communications needs to be done, it will be on clientSocket
	char buffer[4096];		// It's intended to store the data received over a network socket

	while (1)
	{
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
		/*
		It attempts to receive data from a socket.
		ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	| Parameter        | Meaning                                                |
	| ---------------- | ------------------------------------------------------ |
	| `clientSocket`   | The socket file descriptor you're receiving data from  |
	| `buffer`         | The buffer to store the received data                  |
	| `sizeof(buffer)` | Maximum number of bytes to receive (4096 in this case) |
	| `0`              | Flags (0 = no special behavior)                        |

		*/

		if (bytesrecvd <= 0)	// If 0 or -ve recieved from client close the connection
		{
			cout << "Client Disconnected\n";
			break;
		}

		string message(buffer, bytesrecvd);
		cout << "Message from client : " << message << endl;

		for (auto client : clients)
		{
			if (client != clientSocket)
			{
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end())
	{
		clients.erase(it);
	}

	closesocket(clientSocket);
}

int main()
{
	// Initialize Windows Socket
	if (!Initialize())
	{
		cout << "winsock initialization failed\n";
		return 1;
	}
	cout << "server program\n";

	// Create the socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);	// AF_INET is IPV4 address mechanism, SOCK_STREAM is telling to create socket of type TCP, 0 is what protocol we use

	// check if socket is properly created or not
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Socket Creation Failed!\n";
		return 1;
	}

	// create address structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);		// htons->host to network

	// convert to ipaddress (0.0.0.0) put it inside the sin_family in binary format
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1)	// == 1 is success, InetPton is a Windows networking function that converts IP addresses from text (string) format to binary network format.
	{
		cout << "Setting address structure failed\n";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// bind port
	/*
		listenSocket - The socket file descriptor that you want to bind
		reinterpret_cast<sockaddr*>(&serveraddr) - The address structure cast to the generic sockaddr * type
		sizeof(serveraddr) - The size of the address structure in bytes
	*/
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed\n";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// listen
	// The listen() function puts a socket into listening mode, making it ready to accept incoming client connections.
	// SOMAXCONN is maximum queue size
	/*	The Connection Queue
		When clients try to connect :

		Incoming connections wait in a queue if the server is busy
		Queue size is limited to SOMAXCONN(e.g., 128 connections)
		Overflow connections are rejected if queue is full
		accept() removes connections from the front of the queue*/

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cout << "Listen failed\n";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server has started listening on port : " << port << endl;
	vector<SOCKET> clients;		// To store number of clients interacting with this server

	while (1)
	{
		//accept
		/*
		nullptr (first): Normally a pointer to a sockaddr structure that would be filled with the client's address information. Using nullptr means you don't want to retrieve the client's address
		nullptr (second): Normally a pointer to the length of the address structure. Using nullptr means you don't care about the address length
		*/
		// listenSocket will be used to listen to clients but clientSocket is specif to the client it has connected to
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Invalid Client Socket\n";
		}
		clients.push_back(clientSocket);
		thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}
	
	closesocket(listenSocket);
	/*
	🧠 Summary:
	This code:

	Accepts a new client connection.

	Receives a message from the client.

	Prints the received message.

	Cleans up by closing the sockets.

	🔒 It’s a simple one-time client communication handler, useful for basic TCP server tasks.
	*/

	WSACleanup();
	return 0;
}