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
#define PacketSize 1024

/** declare variable wsa **/
WSADATA wsa;
/** declare socket variables – needed for sockets on both client and sever **/
struct sockaddr_in si_other;
SOCKET s;
int slen = sizeof(si_other);

unsigned long noBlock;
//char buffer[BufferLength] = "hello world";

 /**file variable **/
unsigned long fileLen; // length of image file
FILE *fp; // file pointer
char *buffer; // pointer to character array

char fileName[BufferLength] = "test.jpg";
int numPackets;

int main() {

    //OPEN IMAGE FILE AND COPY TO DATA STRUCTURE
    fp = fopen(fileName, "rb");
    
    if (fp == NULL) {
        printf("\n Error Opening Image - read");
        fclose(fp);
        exit(0);
    }

    /*** ALLOCATE MEMORY (BUFFER) TO HOLD IMAGE *****/
    fseek(fp, 0, SEEK_END); //go to EOF
    fileLen = ftell(fp); // determine length
    fseek(fp, 0, SEEK_SET); //reset fp
    buffer = (char*)malloc(fileLen + 1); //allocated memory
    if (!buffer) {
        printf("\n memory error allocating buffer");
        fclose(fp);
        return 1;
    }
   
    /********* READ FILE DATA INTO BUFFER AND CLOSE FILE *************/
    fread(buffer, fileLen, 1, fp);
    fclose(fp);
    printf("\nFile Length:  %d \n", fileLen);
    numPackets = fileLen / PacketSize + 1;
    /****** INITIALIZING WINSOCK ***********/
    printf("\n****** INITIALIZING WINSOCK ***********");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    else printf("\nWINSOCK INITIALIZED");

    /*****  CREATE CLIENT SOCKET  ****/
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    printf("UDP CLIENT SOCKEET CREATED.\n");

    /*****  INITIALIZE SOCKET STRUCT   - Non Blocking Client ****/
    noBlock = 1;
    ioctlsocket(s, FIONBIO, &noBlock);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1"); //current IP address is a dummy address, need to add actual address
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(80);





    std::string fileName = "test.jpg";
    std::ifstream inputFile(fileName, std::ios::binary);

    if (!inputFile.is_open()) {
        std::cerr << "Error Opening file: " << fileName << std::endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    char buffer[BufferLength];

    sendto(s, fileName.c_str(), fileName.length(), 0, (struct sockaddr*)&si_other, sizeof(si_other));
//<<<<<<< HEAD

    for (int i = 1; i <= numPackets; i++) {
        inputFile.read(buffer + sizeof(int), BufferLength - sizeof(int));
        int bytesRead = static_cast<int>(inputFile.gcount());

        memcpy(buffer, &i, sizeof(int));

        sendto(s, buffer, bytesRead + sizeof(int), 0, (struct sockaddr*)&si_other, sizeof(si_other));
        printf("Packet number %d sent.", i);
        int ack = 0;
        while (ack != i) {
            recvfrom(s, (char*)&ack, sizeof(int), 0, nullptr, nullptr);
            if (ack = i) {
                printf("Ack Recieved for packet %d", i);
            }
        }
   }
    
   /* while (!inputFile.eof()) {
        inputFile.read(buffer + sizeof(int), BufferLength - sizeof(int));
        int bytesRead = static_cast<int>(inputFile.gcount());

        memcpy(buffer, &seqNum, sizeof(int));

        sendto(s, buffer, bytesRead + sizeof(int), 0, (struct sockaddr*)&si_other, sizeof(si_other));
        printf("Packet number %d sent.", seqNum);
        int ack;
        recvfrom(s, (char *)&ack, sizeof(int), 0, nullptr, nullptr);

        if (ack != seqNum) {
            --seqNum;
        }
        else {
            seqNum++;
        }
    }
   */
//=======
    inputFile.read(buffer, BufferLength);
    sendto(s, buffer, static_cast<int>(inputFile.gcount()), 0, (struct sockaddr*)&si_other, sizeof(si_other));
    std::cout << "File sent successfully" << std::endl;
//>>>>>>> parent of 06163c2 (sends Acks)

    inputFile.close();
    closesocket(s);
    WSACleanup();


    return 0;
}


