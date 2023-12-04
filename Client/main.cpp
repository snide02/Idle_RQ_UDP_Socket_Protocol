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

 /**file variable **/
unsigned long fileLen; // length of image file
FILE* fp; // file pointer
char* buffer; // pointer to character array

int numPackets;

const float TIMEOUT_SEC = 0.1; //timer to timeout in seconds

int main() {

    /****** INITIALIZING WINSOCK ***********/
    printf("\n****** INITIALIZING WINSOCK ***********");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    else printf("\nWINSOCK INITIALIZED\n");

    //OPEN IMAGE FILE AND COPY TO DATA STRUCTURE
    fp = fopen("test.jpg", "rb");
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

    numPackets = fileLen / PacketSize + 1; //extra packet for the remaining data that doesn't fit into a whole packet.
    printf("Number of Packets: %d\n", numPackets);

    int lastPacketSize = numPackets * PacketSize - BufferLength;

    /*****  CREATE CLIENT SOCKET  ****/
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    printf("UDP CLIENT SOCKEET CREATED.\n");

    /*****  INITIALIZE SOCKET STRUCT   - Non Blocking Client ****/
    noBlock = 1;
    ioctlsocket(s, FIONBIO, &noBlock);

    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");//127.0.0.1 - current IP address is a dummy address, need to add actual address
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
    char packet[PacketSize + 2]; //creating a packet to be sent and accounting for the size of the sequence number.

    sendto(s, fileName.c_str(), fileName.length(), 0, (struct sockaddr*)&si_other, sizeof(si_other));
    inputFile.read(buffer, BufferLength);

    //break file on buffer into packs then send them
    //i in this for loop is our packet number.
    for (int i = 0; i < numPackets; i++) {
        char sequenceNum = (char)i;
        int j = 0; // this j resets to 0 so that we can reassign the value of the packet to new data.
        if (i < 9) { //for packets 1-9
            while (j < PacketSize) {
                //reassigning the packet value to the shifted area in the buffer based upon the packet number which in our case is i
                packet[j] = buffer[j + PacketSize * i];  
                j += 1;
            }
            packet[PacketSize + 1] = i; //Adding the sequence number to the endo f the packet
            printf("\npacket[%d] = %d", PacketSize + 1, packet[PacketSize + 1]);
            sendto(s, packet, PacketSize + 2, 0, (struct sockaddr*)&si_other, sizeof(si_other));
            printf("\nsent packet #%d\n", i);
        }
        if (i == 9) { //for packet 10
            while (j < lastPacketSize) {
                //reassigning the packet value to the shifted area in the buffer based upon the packet number which in our case is i
                packet[j] = buffer[j + PacketSize * i];
                j += 1;
            }
            packet[lastPacketSize + 1] = i; //Adding the sequence number to the endo f the packet
            printf("\npacket[%d] = %d", lastPacketSize + 2, i);
            sendto(s, packet, lastPacketSize + 2, 0, (struct sockaddr*)&si_other, sizeof(si_other));
            printf("\nsent packet #%d\n", i);
        }

        char recievedACK;
        //Sleep(1000);
        float t1 = clock(); // get the current number in ticks 
        float elapsedTime = 0;
        bool timeout = false; //boolean variable to see if we timeout and also used to exit the loop if we receive the ack correctly.
        if (recvfrom(s, &recievedACK, sizeof(char), 0, (struct sockaddr*)&si_other, &slen) == SOCKET_ERROR) {
            //printf("\n recvfrom() failed with error code : %d", WSAGetLastError());
            printf("\nWaiting for the ACK, did not receive right away");
            while (!timeout) {
                float t2 = clock();
                elapsedTime = (float)(t2 - t1) / CLOCKS_PER_SEC; // the amount of time in seconds that have elapsed since we try and receeive the ack and the current moment.
                printf("\ntime elapsed:%f", elapsedTime);
                recvfrom(s, &recievedACK, sizeof(char), 0, (struct sockaddr*)&si_other, &slen); // attempts to receive the ack again.
                Sleep(100); //Delay so that we are not printing every millisecond that the code is running
                if (elapsedTime >= TIMEOUT_SEC) { //Checking if the elapsedtime in seconds is greater than our timer
                    printf("\n Timeout Initialized");
                    printf("\nResending Packet %d", i);
                    i--; // decrementing i so that it resends the current packet (since the for loop automatically increments i)
                    timeout = true; // exitting the while loop because we timed out
                }
                else if (sequenceNum + 48 == recievedACK) {
                    printf("\n ACK %c recieved\n", recievedACK);
                    timeout = true; //exiting the while loop because we received the packet before time out.
                }
            }
        }
        else if (sequenceNum + 48 == recievedACK) {
            printf("\n ACK %c recieved 1st try\n", recievedACK);
        }
    }

    std::cout << "File sent successfully" << std::endl;
    inputFile.close();
    closesocket(s);
    WSACleanup();

    return 0;
}


