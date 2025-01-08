/*****************************************************************************
 *
 *      server.h
 *
 *      Isabel Muste (imuste01)
 *      10/02/2024
 *      
 *      CS 112 HW02
 * 
 *      the server interface can be used to connect clients to a server and
 *      allow for sending messages back and forth between clients and the
 *      server
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <math.h> 
#include <limits.h>
#include <time.h>

#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>



#ifndef SERVER_H


typedef struct __attribute__((__packed__)) 
{
        unsigned short msgType;
        char msgSource[20];
        char msgDest[20];
        unsigned int contentLength;
        unsigned int msgID;
        
} msgHeader;


typedef struct 
{
        int clientSD;
        char *clientID;

        int bufferSize;
        int bufferRead;
        char *readBuffer;

        int headerRead;
        msgHeader *header;

        int contentRead;
        char *msgContent;

        bool clientActive;
        unsigned long long timeAdded;
        
} clientInfo;


typedef struct 
{
        clientInfo *clientList;
        int portNumber;
        int listSize;
        // int nextAvailSlot;
        int numClients;
        unsigned long long initServerTime;

        int listenSD;
        fd_set activeFDSet;
        fd_set readFDSet;

} server;



/*****************************************************************
*                    FUNCTION DECLARATIONS
*****************************************************************/
server *newServer(int port);

void runChatApp(server *theServer);
void processClient(server *theServer, int clientSD);
void processClientData(server *theServer, int slot, bool clientActive);

// Client Message Processing
void clientReadCall(server *theServer, int clientSlot);
int readClientHeader(server *theServer, int clientSlot);
void processHeaderInfo(server *theServer, int clientSlot);
bool processChatMessage(server *theServer, int clientSlot);
int readClientMessage(server *theServer, int clientSlot);

// Server to Client Communication
void sendHelloAck(server *theServer, int clientSlot);
void sendClientList(server *theServer, int clientSlot);
void forwardClientMessage(server *theServer, int clientSlot, 
        msgHeader forwardHeader, int clientSD);
void sendAlreadyPresent(server *theServer, int clientSlot);
void sendCannotDeliver(server *theServer, int clientSlot);

// Checking Functions
bool checkHeaderValid(server *theServer, int clientSlot);
bool checkHelloMsg(server *theServer, int clientSlot);
bool checkHelloHeader(server *theServer, int clientSlot);
bool checkSpaceInClientList(server *theServer, int clientSlot);
bool checkIDExists(server *theServer, int clientSlot);
bool checkOnlyWhiteSpace(char* String);
void checkNegOneError(int returnVal);
void checkReturnVal(int returnVal, server *theServer, int clientSlot);
void checkAllocError(char *memAllocated);

// Client List Organization
int reorderClientList(server *theServer, int clientSlot);
void shiftClients(server *theServer, int moveSpot);
void makeClientInfoCopy1(clientInfo *fromClient, clientInfo *toClient);
void makeClientInfoCopy2(clientInfo *fromClient, clientInfo *toClient);

// Remove / Reset Functions
void removeClient(server *theServer, int clientSlot);
bool removeExpiredClients(server *theServer);
void clearListSlot(server *theServer, int clientSlot);
void clearMsgData(server *theServer, int clientSlot);
void clearBuffer(server *theServer, int clientSlot);

// Helper Functions
void createSocket(server *theServer);
long long getTimeoutValue(server *theServer);
unsigned long long getInitialTime();
unsigned long long getCurrTime(server *theServer);

// void printHeader(msgHeader *header);
// void printClientList(server *theServer);


#endif