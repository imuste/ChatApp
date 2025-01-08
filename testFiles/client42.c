/*****************************************************************************
 *
 *      client.c
 *
 *      Isabel Muste (imuste01)
 *      10/02/2024
 *      
 *      CS 112 HW02
 * 
 *      ...
 *      
 *
 *****************************************************************************/

#include <math.h> 
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct __attribute__((__packed__)) 
{
        unsigned short msgType;
        char msgSource[20];
        char msgDest[20];
        unsigned int contentLength;
        unsigned int msgID;
        
} msgHeader;


int setupServerConn();

void writeHello1(int clientSD);
void writeHello2(int clientSD);
void readHelloAck(int clientSD);
void readClientList(int clientSD);
void readClientExistsErr(int clientSD);

void printHeader(msgHeader header);
void printClientList(char *clientList);

/*
 * name:      main
 * purpose:   opens the commands file and initializes a new cache instance
 * arguments: argc, argv
 * returns:   exit success
 * effects:   none
 */
int main(int argc, char *argv[])
{       
        int clientSD = setupServerConn();

        //write the HELLO message into the socket
        writeHello1(clientSD);

        //read HELLO ACK from the socket
        readHelloAck(clientSD);

        //read CLIENT LIST from the socket
        readClientList(clientSD);

        writeHello2(clientSD);

        char buffer[50];
        int returnVal = read(clientSD, buffer, sizeof(buffer));
        if (returnVal == 0) {
                printf("client was successfully removed\n");
                exit(EXIT_SUCCESS);
        }

        // readClientExistsErr(clientSD);


        //close the socket
        shutdown(clientSD, SHUT_RDWR);
        close(clientSD);
        
        return EXIT_SUCCESS;
}


int setupServerConn()
{
        struct sockaddr_in clientAddress, proxyAddress;
        int clientSD = socket(AF_INET, SOCK_STREAM, 0);
        assert(clientSD != -1);

        //ensure to close the socket and port after termination
        int opt = 1;
        setsockopt(clientSD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        //Client setup to bind the socket to port 9099 and any IP address
        //this is the setup specifying the client info
        memset(&clientAddress, 0, sizeof(struct sockaddr_in));
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        // clientAddress.sin_port = htons(9095);
        int returnVal = bind(clientSD, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
        assert(returnVal != -1);

        //Client setup to connect the socket to the proxy and its IP address
        //this is the setup specifying the proxy info
        memset(&proxyAddress, 0, sizeof(struct sockaddr_in));
        proxyAddress.sin_family = AF_INET;
        proxyAddress.sin_addr.s_addr = inet_addr("10.4.2.18");
        proxyAddress.sin_port = htons(9099);
        returnVal = connect(clientSD, (struct sockaddr *)&proxyAddress, sizeof(proxyAddress));
        assert(returnVal != -1);

        return clientSD;
}


void writeHello1(int clientSD)
{
        msgHeader header1;
        header1.msgType = htons(1);
        strcpy(header1.msgSource, "client42");
        header1.msgSource[sizeof(header1.msgSource) - 1] = '\0';
        strcpy(header1.msgDest, "Server");
        header1.msgDest[sizeof(header1.msgDest) - 1] = '\0';
        header1.contentLength = htonl(0);
        header1.msgID = htonl(0);
        int returnVal = write(clientSD, (char *)&header1, sizeof(msgHeader));
        assert(returnVal != -1);
}


void readHelloAck(int clientSD)
{
        msgHeader receivedHeader;
        int bufferSize = sizeof(msgHeader);
        int returnVal = read(clientSD, (char *)&receivedHeader, bufferSize);
        assert(returnVal != -1);

        receivedHeader.msgType = ntohs(receivedHeader.msgType);          
        receivedHeader.contentLength = ntohl(receivedHeader.contentLength);
        receivedHeader.msgID = ntohl(receivedHeader.msgID);

        printHeader(receivedHeader);
}


void readClientList(int clientSD)
{
        msgHeader receivedHeader;
        int bufferSize = sizeof(msgHeader) + 400;
        char *clientListBuffer = malloc(bufferSize);
        int returnVal = read(clientSD, clientListBuffer, bufferSize);
        assert(returnVal != -1);
        memcpy(&receivedHeader, clientListBuffer, sizeof(msgHeader));

        receivedHeader.msgType = ntohs(receivedHeader.msgType);          
        receivedHeader.contentLength = ntohl(receivedHeader.contentLength);
        receivedHeader.msgID = ntohl(receivedHeader.msgID);

        printHeader(receivedHeader);
        printClientList(clientListBuffer + sizeof(msgHeader));
}



void writeHello2(int clientSD)
{
        msgHeader header1;
        header1.msgType = htons(1);
        strcpy(header1.msgSource, "client38");
        header1.msgSource[sizeof(header1.msgSource) - 1] = '\0';
        strcpy(header1.msgDest, "Server");
        header1.msgDest[sizeof(header1.msgDest) - 1] = '\0';
        header1.contentLength = htonl(0);
        header1.msgID = htonl(0);
        int returnVal = write(clientSD, (char *)&header1, sizeof(msgHeader));
        assert(returnVal != -1);
}



void readClientExistsErr(int clientSD)
{
        msgHeader receivedHeader;
        int bufferSize = sizeof(msgHeader);
        int returnVal = read(clientSD, (char *)&receivedHeader, bufferSize);
        assert(returnVal != -1);

        receivedHeader.msgType = ntohs(receivedHeader.msgType);          
        receivedHeader.contentLength = ntohl(receivedHeader.contentLength);
        receivedHeader.msgID = ntohl(receivedHeader.msgID);

        printHeader(receivedHeader);
}



void printHeader(msgHeader header) 
{
        printf("\n");
        if (header.msgType == 2) {
                printf("HELLO ACK\n");
        }
        else if (header.msgType == 4) {
                printf("CLIENT LIST\n");
        }
        else if (header.msgType == 5) {
                printf("CHAT\n");
        }
        else if (header.msgType == 7) {
                printf("ERROR_CLIENT_ALREADY_PRESENT\n");
        }
        else if (header.msgType == 8) {
                printf("ERROR_CANNOT_DELIVER\n");
        }
        printf("Message Header:\n");
        printf("  msgType: %u\n", header.msgType);
        printf("  msgSource: %s\n", header.msgSource);
        printf("  msgDest: %s\n", header.msgDest);
        printf("  contentLength: %u\n", header.contentLength);
        printf("  msgID: %u\n", header.msgID);
        printf("\n");
}

void printClientList(char *clientList)
{
        char *currentString = clientList;
        while (*currentString != '\0') {
                printf("%s\n", currentString);
                currentString += strlen(currentString) + 1;
        }
}