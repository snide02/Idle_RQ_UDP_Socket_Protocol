#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BufferLength 9319 //our buffer length
#define PACKETSIZE 1024

int receiveIntFrom(SOCKET serverSocket, sockaddr_in& clientAddr) {
    int value;
    int clientAddrSize = sizeof(clientAddr);
    recvfrom(serverSocket, reinterpret_cast<char*>(&value), sizeof(int), 0,
        reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    return value;
}

void sendIntTo(SOCKET serverSocket, const sockaddr_in& clientAddr, int value) {
    sendto(serverSocket, reinterpret_cast<const char*>(&value), sizeof(int), 0,
        reinterpret_cast<const sockaddr*>(&clientAddr), sizeof(clientAddr));
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    // Below is where we change code, do not change anything above




    sockaddr_in clientAddr;
    int receivedData = receiveIntFrom(serverSocket, clientAddr);
    std::cout << "Received data from client: " << receivedData << std::endl;

    // Echo back the received data to the client
    sendIntTo(serverSocket, clientAddr, receivedData);








    closesocket(serverSocket);
    WSACleanup();
    return 0;
}