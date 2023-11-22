#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BufferLength 1024 //our buffer length
#define PACKETSIZE 1024
/** declare variable wsa **/
WSADATA wsa;
/** declare socket variables � needed for sockets on both client and sever **/
struct sockaddr_in server;
SOCKET s;
int slen = sizeof(server);
int recv_len;
unsigned long noBlock;
char buffer[BufferLength];

char fileName[9320];

int main() {

    /****** INITIALIZING WINSOCK ***********/
    printf("\n****** INITIALIZING WINSOCK ***********");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    else printf("\nWINSOCK INITIALIZED");

    /*****  CREATE SERVER SOCKET  ****/
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    else printf("\nUDP SERVER SOCKET CREATED");
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // or INADDR_ANY
    server.sin_family = AF_INET;
    server.sin_port = htons(80);

    /***** BIND SOCKET ****/
    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("\nSERVER SOCKET BIND SUCCESS");

    /***** WAIT FOR DATA ****/
    printf("\nWaiting for data...");

    //buffer = (char*)malloc((fileLen + 1)); //allocated mmmory
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buffer, '\0', BufferLength);

		//try to receive some data, this is a blocking call
        recvfrom(s, fileName, 9320, 0, (struct sockaddr*)&server, &slen);

		if ((recv_len = recvfrom(s, buffer, BufferLength, 0, (struct sockaddr*)&server, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));
		printf("Data: %s\n", buffer);

		//now reply the client with the same data
		if (sendto(s, buffer, recv_len, 0, (struct sockaddr*)&server, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

    return 0;
}