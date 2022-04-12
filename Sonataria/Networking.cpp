#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include "Logger.h"
#include "Networking.h"
#include <winsock2.h>
#include "HTTPRequest.hpp"
#include "UserData.h"
using namespace std;

Networking network;

string convertToString(char* a, int size);
bool readfile(SOCKET sock, FILE* f);
bool readdata(SOCKET sock, void* buf, int buflen);
bool readlong(SOCKET sock, long* value);

#pragma comment (lib, "WS2_32.lib")

// The IP and PORT to connect to
const char IP[] = "47.41.144.77";
const int PORT = 57015;

/**
 * Default constructor.
 * 
 */
Networking::Networking() {
	// Set the version of the game
	this->version = "20220412-J-01";

	// Set the information for connecting to the server
	this->ServerAddress = "http://127.0.0.1:3000";
}

/**
 * Default deconstructor.
 * 
 */
Networking::~Networking() {

}

/**
 * Get the version of the game.
 * 
 * @return the game version
 */
string Networking::getLocalVersion() {
	return this->version;
}

bool Networking::GetProfileData(string cardID) {
	try {
		// Build the URL for the connection
		string URL = this->ServerAddress + "/data/profile/" + cardID;
		http::Request request{ URL };

		logger.log("Attempting to retrieve profile data... [" + URL + "]");

		// Set a GET Request
		const auto response = request.send("GET", "", {}, std::chrono::seconds(5));

		if (response.status.code == http::Status::Ok) {
			// Store the response into data
			string data = string{ response.body.begin(), response.body.end() };

			// Set the user settings
			userData.setUserData(data);

			return true;
		}
		else {
			logger.logError("Invalid HTTP Status Code: " + to_string(response.status.code));
			return false;
		}
	}
	catch (const std::exception& e) {
		logger.logError("Login Request Failed: ");
		logger.logError(e.what());
		return false;
	}
}

/**
 * Check the server for it's status.
 * 
 * @return the status of the server
 */
int Networking::checkConnection() {
	return 2;
	WSADATA wsaData;
	SOCKET s;
	sockaddr_in server;

	logger.log(L"Initializing Winsock...");
	// Initialize WinSock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wstring errorMsg = L"Failed. Error code: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return 2;
	}

	logger.log(L"Winsock Initialized.");

	// Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		wstring errorMsg = L"Could not create socket: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return 2;
	}

	logger.log(L"Socket created.");

	// Set the server connection data
	server.sin_addr.S_un.S_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	// Attempt to connect to the server
	if (connect(s, (sockaddr*)&server, sizeof(server)) < 0) {
		logger.logError(L"Could not connect to server.");
		WSACleanup();
		return 2;
	}

	logger.log(L"Connected.");

	char SEND[] = "Status";
	// Send data to the server asking for the status
	if (send(s, SEND, strlen(SEND), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return 2;
	}

	logger.log(L"Bytes sent.");

	const int recvBuffSize = 50;
	char recvBuffer[recvBuffSize] = "";
	// Try to receive the server status
	if (recv(s, recvBuffer, recvBuffSize, 0) <= 0) {
		logger.logError(L"Failed to receive data.");
		closesocket(s);
		WSACleanup();
		return 2;
	}

	//cout << "RECEIVED CONN: [" << recvBuffer << "]" << endl;
	logger.log(L"Received connection.");

	string sStatus;
	sStatus = (char)recvBuffer[0];
	int status = stoi(sStatus);
	
	char SEND2[] = "EndConn";
	// Send that we are done with the server
	if (send(s, SEND2, strlen(SEND2), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return 2;
	}
	
	logger.log(L"Bytes sent.");

	//1-online 2-offline 3-maint

	// Close the socket
	if (closesocket(s) == SOCKET_ERROR) {
		logger.logError(L"Could not close socket.");
		WSACleanup();
		return 2;
	}

	logger.log(L"Socket Closed.");

	// Clean up and return the status
	WSACleanup();
	return status;
}

/**
 * Check the server if there are updates availible.
 * 
 * @return the version stored on the server
 */
string Networking::checkForUpdates() {

	WSADATA wsaData;
	SOCKET s;
	sockaddr_in server;

	logger.log(L"Initializing Winsock...");
	// Initialize WinSock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wstring errorMsg = L"Failed. Error code: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Initialized.");

	// Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		wstring errorMsg = L"Could not create socket: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Socket created.");

	// Set the info for the connection
	server.sin_addr.S_un.S_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	// Attempt to connect
	if (connect(s, (sockaddr*)&server, sizeof(server)) < 0) {
		logger.logError(L"Could not connect to server.");
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Connected.");

	char SEND[] = "Version";
	// Send data asking for the version
	if (send(s, SEND, strlen(SEND), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Bytes sent.");

	const int recvBuffSize = 50;
	char recvBuffer[recvBuffSize] = "";
	// Receive the version
	if (recv(s, recvBuffer, recvBuffSize, 0) <= 0) {
		logger.logError(L"Failed to receive data.");
		closesocket(s);
		WSACleanup();
		return "NONE";
	}

	string sVersion = convertToString(recvBuffer, recvBuffSize);

	//cout << "RECEIVED VER: [" << sVersion << "]" << endl;
	logger.log(L"Received Version.");

	char SEND2[] = "EndConn";
	// Send that we are done with connection
	if (send(s, SEND2, strlen(SEND2), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Bytes sent.");

	// Close the socket
	if (closesocket(s) == SOCKET_ERROR) {
		logger.logError(L"Could not close socket.");
		WSACleanup();
		return "NONE";
	}

	logger.log(L"Socket Closed.");

	// Close the connection and return the version
	WSACleanup();
	return sVersion;
}

/**
 * Download the update.
 * 
 * @return true if successful
 */
bool Networking::downloadUpdate() {
	WSADATA wsaData;
	SOCKET s;
	sockaddr_in server;

	logger.log(L"Downloading...");

	logger.log(L"Initializing Winsock...");
	// Initialize WinSock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		wstring errorMsg = L"Failed. Error code: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return false;
	}

	logger.log(L"Initialized.");

	// Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		wstring errorMsg = L"Could not create socket: " + WSAGetLastError();
		logger.logError(errorMsg);
		WSACleanup();
		return false;
	}

	logger.log(L"Socket created.");

	// Set data for the connection
	server.sin_addr.S_un.S_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	// Attempt to connect
	if (connect(s, (sockaddr*)&server, sizeof(server)) < 0) {
		logger.logError(L"Could not connect to server.");
		WSACleanup();
		return false;
	}

	logger.log(L"Connected.");

	char SEND[] = "DLUpdate";
	// Send that we want to download the update
	if (send(s, SEND, strlen(SEND), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return false;
	}

	logger.log(L"Bytes sent.");

	//Create a new file that we will write to in binary
	FILE* filehandle = fopen("C:/SNA_UPDATE/Update.zip", "wb+");
	if (filehandle != NULL)
	{
		// Read from the server and save to the file
		bool ok = readfile(s, filehandle);
		fclose(filehandle);

		if (ok)
		{
			logger.log(L"Download success!");
		}
		else {
			logger.logError(L"Error Downloading File!");
			return false;
		}
	}

	char SEND2[] = "EndConn";
	// Send that we are done with the connection
	if (send(s, SEND2, strlen(SEND2), 0) == SOCKET_ERROR) {
		logger.logError(L"Could not send message.");
		closesocket(s);
		WSACleanup();
		return false;
	}

	logger.log(L"Bytes sent.");

	// Close the socket
	if (closesocket(s) == SOCKET_ERROR) {
		logger.logError(L"Could not close socket.");
		WSACleanup();
		// don't return false as not a fatal error
		// still success in downloading the update file
	}
	else {
		logger.log(L"Socket Closed.");
	}

	// Clean up and return that we downloaded successfully
	WSACleanup();

	logger.log(L"Done downloading...");

	return true;
}

/**
 * Convert the results to a string.
 * 
 * @param a char[]
 * @param size size of array
 * @return the string
 */
string convertToString(char* a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++) {
		if (a[i] == ' ' || a[i] == '\0') { }
		else {
			s = s + a[i];
		}
	}
	return s;
}

/**
 * Read data in from a file.
 * 
 * @param sock Socket to connect to
 * @param f File to read from
 * @return true if successful
 */
bool readfile(SOCKET sock, FILE* f)
{
	long filesize;
	if (!readlong(sock, &filesize))
		return false;
	if (filesize > 0)
	{
		char buffer[1024];
		do
		{
			int num = min(filesize, sizeof(buffer));
			if (!readdata(sock, buffer, num))
				return false;
			int offset = 0;
			do
			{
				size_t written = fwrite(&buffer[offset], 1, num - offset, f);
				if (written < 1)
					return false;
				offset += written;
			} while (offset < num);
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
}

/**
 * Not my function, used for readFile()
 * 
 * @param sock
 * @param buf
 * @param buflen
 * @return 
 */
bool readdata(SOCKET sock, void* buf, int buflen)
{
	char* pbuf = (char*)buf;

	while (buflen > 0)
	{
		int num = recv(sock, pbuf, buflen, 0);
		if (num == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// optional: use select() to check for timeout to fail the read
				continue;
			}
			return false;
		}
		else if (num == 0)
			return false;

		pbuf += num;
		buflen -= num;
	}

	return true;
}

/**
 * Not my function, used for readFile()
 *
 * @param sock
 * @param buf
 * @param buflen
 * @return
 */
bool readlong(SOCKET sock, long* value)
{
	if (!readdata(sock, value, sizeof(value)))
		return false;
	*value = ntohl(*value);
	return true;
}