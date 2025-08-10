#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

/*
	initialize winsock
	create socket
	connect to the server
	send/receive
	close the socket
*/

bool Initialize()
{
	WSAData data;
	return WSAStartup(MAKEWORD(2,2), &data) == 0;
}

void SendMsg(SOCKET s)
{
	cout << "Enter your chat name : " << endl;
	string name;
	getline(cin, name);
	string message;

	while (1)
	{
		getline(cin, message);
		string msg = name + " : " + message;
		int bytesent = send(s, msg.c_str(), msg.length(), 0);
		if (bytesent == SOCKET_ERROR)
		{
			cout << "Error sending message\n";
			break;
		}

		if (message == "quit" or message == "QUIT" or message == "Quit")
		{
			cout << "Stopping the application\n";
			break;
		}
	}

	closesocket(s);
	WSACleanup();

}

void ReceiveMsg(SOCKET s)
{
	char buffer[4096];
	int recvLength;
	string msg = "";
	while (1)
	{
		recvLength = recv(s, buffer, sizeof(buffer), 0);
		if (recvLength <= 0)
		{
			cout << "Disconnected from server\n";
			break;
		}
		else
		{
			msg = string(buffer, recvLength);
			cout << msg << endl;
		}
	}

	closesocket(s);
	WSACleanup();
}

int main()
{
	if (!Initialize())
	{
		cout << "Initialize winsock failed\n";
		return 1;
	}
	cout << "Client Program Started\n";
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		cout << "Invalid Socket Created\n";
		return 1;
	}

	sockaddr_in serveraddr;
	int PORT = 12345;
	string serveraddress = "127.0.0.1";
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));
	if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR)
	{
		cout << "Not able to connect to server\n";
		cout << ": " << WSAGetLastError();
		closesocket(s);
		WSACleanup();
		return 1;
	}

	cout << "Successfully connected to server\n";

	thread senderThread(SendMsg, s);
	thread receiverThread(ReceiveMsg, s);

	senderThread.join();
	receiverThread.join();

	return 0;
}