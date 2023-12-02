#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "main.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

#define BufferLength 9319
#define PACKETSIZE 1024

int receiveIntFrom(SOCKET clientSocket, sockaddr_in& serverAddr) {
    int value;
    int serverAddrSize = sizeof(serverAddr);
    recvfrom(clientSocket, reinterpret_cast<char*>(&value), sizeof(int), 0,
        reinterpret_cast<sockaddr*>(&serverAddr), &serverAddrSize);
    return value;
}

void sendIntTo(SOCKET clientSocket, const sockaddr_in& serverAddr, int value) {
    sendto(clientSocket, reinterpret_cast<const char*>(&value), sizeof(int), 0,
        reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr));
}
struct Packet {
    int seqNum;
    char data[PACKETSIZE];
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(12345);

    // Below is where we change code, do not change anything above




    int dataToSend = 123; // Replace with the actual value you want to send
    sendIntTo(clientSocket, serverAddr, dataToSend);

    // Receive the echoed data from the server
    sockaddr_in receivedFromAddr;
    int receivedData = receiveIntFrom(clientSocket, receivedFromAddr);

    std::cout << "Received echoed data from server: " << receivedData << std::endl;








    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

