#include <iostream>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

char ip[10] = "127.0.0.1";
int port = 8888;
const short BUFFER_SIZE = 1024;
std::string recipient_folder = "C:\\recipientfiles\\";
std::string sender_folder = "C:\\senderfiles\\";
std::string working_directory;

int send_file() {
	// Key variables for all program
	int erStat;										// For checking errors in sockets functions

	//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
	in_addr ip_to_num;
	inet_pton(AF_INET, ip, &ip_to_num);


	// WinSock initialization
	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		cout << "Error WinSock version initializaion #";
		cout << WSAGetLastError();
		return 1;
	}
	else
		cout << "WinSock initialization is OK" << endl;

	// Socket initialization
	SOCKET сlientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (сlientSocket == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(сlientSocket);
		WSACleanup();
	}
	else
		cout << "Client socket initialization is OK" << endl;

	// Establishing a connection to Server
	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(port);

	erStat = connect(сlientSocket, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
		closesocket(сlientSocket);
		WSACleanup();
		return 1;
	}
	else
		cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;

	std::string filename;
	std::string path;
	do {
		cout << "Enter filename" << endl;
		std::cin >> filename;
		path = sender_folder + filename;

	} while (!filesystem::exists(path) && !filesystem::is_regular_file(path));

	send(сlientSocket, filename.c_str(), filename.size(), 0);

	std::ifstream inputFile(path, std::ios::binary);
	if (!inputFile.is_open()) {
		std::cerr << "Error opening file for reading." << std::endl;
		closesocket(сlientSocket);
		WSACleanup();
		return 1;
	}
	char buffer[BUFFER_SIZE];
	int bytesRead;
	do {
		inputFile.read(buffer, BUFFER_SIZE);
		bytesRead = inputFile.gcount();
		if (bytesRead > 0) {
			send(сlientSocket, buffer, bytesRead, 0);
		}
	} while (inputFile.good());
	cout << "File sent successfully" << endl;
	closesocket(сlientSocket);
	WSACleanup();

	return 0;
}

int get_file() {
	int erStat;								// Keeps socket errors status

	//IP in string format to numeric format for socket functions. Data is in "ip_to_num"
	in_addr ip_to_num;
	erStat = inet_pton(AF_INET, ip, &ip_to_num);

	if (erStat <= 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		return 1;
	}
	// WinSock initialization
	WSADATA wsData;

	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		cout << "Error WinSock version initializaion #";
		cout << WSAGetLastError();
		return 1;
	}
	else
		cout << "WinSock initialization is OK" << endl;

	// Server socket initialization
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else
		cout << "Server socket initialization is OK" << endl;

	// Server socket binding
	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));	// Initializing servInfo structure

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(port);

	erStat = bind(serverSocket, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else
		cout << "Binding socket to Server info is OK" << endl;

	//Starting to listen to any Clients
	erStat = listen(serverSocket, SOMAXCONN);

	if (erStat != 0) {
		cout << "Can't start to listen to. Error # " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Listening..." << endl;
	}

	sockaddr_in clientInfo;
	ZeroMemory(&clientInfo, sizeof(clientInfo));	// Initializing clientInfo structure

	int clientInfo_size = sizeof(clientInfo);

	SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientInfo, &clientInfo_size);

	if (clientSocket == INVALID_SOCKET) {
		cout << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
		closesocket(serverSocket);
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	else {
		cout << "Connection to a client established successfully" << endl;
		char clientIP[22];

		inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);	// Convert connected client's IP to standard string format

		cout << "Client connected with IP address " << clientIP << endl;

	}

	char filenameBuffer[BUFFER_SIZE];
	int filenameBytesReceived = recv(clientSocket, filenameBuffer, 1024, 0);
	if (filenameBytesReceived <= 0) {
		std::cerr << "Error receiving file name." << std::endl;
		closesocket(clientSocket);
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::string filename(filenameBuffer, filenameBytesReceived);
	std::string path = recipient_folder + filename;

	char buffer[BUFFER_SIZE];
	int bytesReceived;
	std::ofstream outputFile(path, std::ios::binary);
	if (!outputFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		closesocket(clientSocket);
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	do {
		bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
		if (bytesReceived > 0) {
			outputFile.write(buffer, bytesReceived);
		}
	} while (bytesReceived > 0);

	outputFile.close();
	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();

	std::cout << "File " << filename << " received successfully." << std::endl;

	return 0;
}

void list_files() {
	cout << "List all files in " << working_directory << endl;
	for (const auto& entry : std::filesystem::directory_iterator(working_directory)) {
		if (filesystem::is_regular_file(entry.path())) {
			cout << entry.path().filename() << endl;
		}
	}
}

int main() {
	setlocale(LC_ALL, "ru");

	do {
		cout << "Enter path to working directory" << endl;
		cin >> working_directory;
	} while (!filesystem::exists(working_directory) && !filesystem::is_directory(working_directory));


	while (true) {
		int choice;
		cout << "1) Get File" << endl;
		cout << "2) Send File" << endl;
		cout << "3) List all files in directory" << endl;
		cin >> choice;

		switch (choice) {
		case 1:
			get_file();
			break;
		case 2:
			send_file();
			break;
		case 3:
			list_files();
			break;
		}
	}


}