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
/** declare variable wsa **/
WSADATA wsa;
/** declare socket variables – needed for sockets on both client and sever **/
struct sockaddr_in server;
SOCKET s;
int slen = sizeof(server);
int recv_len;
unsigned long noBlock;
char buffer[BufferLength];
char NewFile[BufferLength];
int NewFileLength = 0;
//char fileLen[9320];


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

    server.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1"); // or INADDR_ANY
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
    fflush(stdout);

    //clear the buffer by filling null, it might have previously received data
    memset(buffer, '\0', BufferLength);

    //try to receive some data, this is a blocking call

    recvfrom(s, buffer, BufferLength, 0, (struct sockaddr*)&server, &slen);
    std::string filename = buffer;
    std::ofstream outputFile(filename, std::ios::binary);
    int count = 0;

    int lastPacketSize = 10 * PACKETSIZE - BufferLength;
    //number 10 here is the amount of packets we are receiving  
    while (count < 10)
    {
        printf("\nWaiting for data...");
        fflush(stdout);

        //clear the buffer by filling null, it might have previously received data
        memset(buffer, '\0', BufferLength);

        //recieve packet from client
        recv_len = recvfrom(s, buffer, PACKETSIZE + 2, 0, (struct sockaddr*)&server, &slen);
        printf("\nReceived packet from %s:%d", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

        int seqNum;

        char recievedSequenceNumb = (char)buffer[recv_len - 1] + 48;
        printf("\nthe sequence number received is %c", recievedSequenceNumb);

        if (recievedSequenceNumb == (char)count + 48) {

            /**** COMBINE PACKET BITS ****/
            NewFileLength = NewFileLength + recv_len;
            int i = 0;

            //for packet 1-9
            if (count < 9) {
                while (i < PACKETSIZE) {
                    seqNum = buffer[PACKETSIZE + 1];

                    NewFile[i + count * PACKETSIZE] = buffer[i];
                    i++;
                }
                printf("\nPacket Length %d \n", recv_len);
            }
            //for packet 10
            if (count == 9) {
                seqNum = buffer[lastPacketSize + 1];

                while (i < 10 * PACKETSIZE - BufferLength) {
                    NewFile[i + count * PACKETSIZE] = buffer[i];
                    i++;
                }
                printf("\nPacket Length %d \n", recv_len);
            }
            printf("\nSequence Number: %d", seqNum);
            printf("\nCount: %d", count);
            //Sleep(1000); // code to test time out by delaying the time it sends the ack back
            if (sendto(s, &recievedSequenceNumb, 1, 0, (struct sockaddr*)&server, slen) == SOCKET_ERROR) {

                printf("\n sendto() failed with error code: %d", WSAGetLastError());
                exit(EXIT_FAILURE);
            }
            count++;
        }
        else
            printf("\nwrong sequence number received %c", recievedSequenceNumb);
    }

    //reaseble packet 
    printf("File Length %d", NewFileLength);
    if (NewFileLength > 0)
    {
        outputFile.write(NewFile, NewFileLength);
        printf("\nFile received and saved");
    }
    else
        printf("error receiving file");


    closesocket(s);
    WSACleanup();

    return 0;
}