/*****************************************************************************
 *
 *      server.c
 *
 *      Isabel Muste (imuste01)
 *      10/02/2024
 *      
 *      CS 112 HW02
 * 
 *      this file contains the function definitions for the server.h interface
 *      
 *
 *****************************************************************************/

#include "server.h"


/*****************************************************************************
*                               SERVER SETUP
******************************************************************************/

/*
 * name:      newServer
 * purpose:   allocates memory for a new server instance
 * arguments: the server port number specified by the user
 * returns:   the server instance
 * effects:   none
 */
server *newServer(int port)
{
        int clientListSize = 500;

        server *theServer = malloc(sizeof(server));
        checkAllocError((char *)theServer);

        theServer->clientList = (clientInfo *)malloc(clientListSize * sizeof(clientInfo));
        checkAllocError((char *)theServer->clientList);

        for (int i = 0; i < clientListSize; i++) {
                theServer->clientList[i].clientSD = -1;
                theServer->clientList[i].clientID = malloc(20);
                checkAllocError(theServer->clientList[i].clientID);

                theServer->clientList[i].bufferSize = 0;
                theServer->clientList[i].bufferRead = 0;
                theServer->clientList[i].readBuffer = NULL;

                theServer->clientList[i].headerRead = 0;
                theServer->clientList[i].header = 
                        (msgHeader *)malloc(sizeof(msgHeader));
                checkAllocError((char *)theServer->clientList[i].header);

                theServer->clientList[i].contentRead = 0;
                theServer->clientList[i].msgContent = malloc(400);
                checkAllocError(theServer->clientList[i].msgContent);

                theServer->clientList[i].clientActive = false;
                theServer->clientList[i].timeAdded = -1;
        }

        theServer->portNumber = port;
        theServer->listSize = clientListSize;
        theServer->numClients = 0;
        theServer->initServerTime = getInitialTime();
        theServer->listenSD = 0;
        
        return theServer;
}


/*
 * name:      runChatApp
 * purpose:   listens for incoming clients and processes their messages
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
void runChatApp(server *theServer)
{
        //setup the server socket
        createSocket(theServer);

        //listen for incoming connections
        int returnVal = listen(theServer->listenSD, 500);
        checkNegOneError(returnVal);
        
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength;
        struct timeval timing;
        FD_ZERO(&theServer->activeFDSet);
        FD_ZERO(&theServer->readFDSet);
        FD_SET(theServer->listenSD, &theServer->activeFDSet);   
        int maxFD = theServer->listenSD;    

        while (true) {
                removeExpiredClients(theServer);
                theServer->readFDSet = theServer->activeFDSet;

                long long timeOutMS = getTimeoutValue(theServer);
                timing.tv_sec = timeOutMS / 1000000;
                timing.tv_usec = timeOutMS % 1000000;
                returnVal = select(maxFD + 1, &theServer->readFDSet, NULL, 
                                NULL, &timing);
                checkNegOneError(returnVal);
                if (returnVal == 0) {
                        continue;
                }

                if (FD_ISSET(theServer->listenSD, &theServer->readFDSet)) {
                        clientAddressLength = sizeof(clientAddress);
                        int clientSD = accept(theServer->listenSD, 
                                (struct sockaddr *)&clientAddress, 
                                &clientAddressLength);
                        if (clientSD == -1) {
                                continue;
                        }
                        FD_SET(clientSD, &theServer->activeFDSet);
                        if (clientSD > maxFD) {
                                maxFD = clientSD;
                        }
                        continue;
                }
                for (int i = 0; i <= maxFD; i++) {
                        if (FD_ISSET(i, &theServer->readFDSet)) {
                                processClient(theServer, i);
                        }
                }
        }
        close(theServer->listenSD);
}




/*****************************************************************************
*                        FUNCTIONALITY FOR EACH CLIENT
******************************************************************************/

/*
 * name:      processClient
 * purpose:   checks if the client exists and either adds the client, or 
 *            continues processing the client message
 * arguments: the server instance, the client socket descriptor
 * returns:   -1 if a client message was fully read, 0 if part of the 
 *            message was read
 * effects:   new client: populates clientSD, timeAdded fields
 */
void processClient(server *theServer, int clientSD)
{
        bool newClient = true;
        int clientSlot = theServer->numClients;
        for (int i = 0; i < theServer->numClients; i++) {
                if (theServer->clientList[i].clientSD == clientSD) {
                        newClient = false;
                        clientSlot = i;
                }
        }

        //the client is new, set SD
        if (newClient) {
                theServer->clientList[clientSlot].clientSD = clientSD;
                theServer->numClients++;
                processClientData(theServer, clientSlot, false);
        }
        else {
                if (!theServer->clientList[clientSlot].clientActive) {
                        processClientData(theServer, clientSlot, false);
                }
                else {
                        processClientData(theServer, clientSlot, true);
                }
        }
}


/*
 * name:      processClientData
 * purpose:   reads data from the socket and proesses this data
 * arguments: the server instance, the client slot, boolean indicating if the 
 *            client is active
 * returns:   none
 * effects:   if client data is not valid, disconnects the client
 */
void processClientData(server *theServer, int slot, bool clientActive)
{
        clientReadCall(theServer, slot);
        clientInfo *theInfo = &theServer->clientList[slot];
        while (theInfo->bufferRead < theInfo->bufferSize) {
                theServer->clientList[slot].timeAdded = getCurrTime(theServer);
                if (theInfo->headerRead < sizeof(msgHeader)) {
                        int headerRead = readClientHeader(theServer, slot);
                        if (headerRead < sizeof(msgHeader)) {
                                break;
                        }
                }
                if (!clientActive && checkHelloMsg(theServer, slot)) {
                        slot = reorderClientList(theServer, slot);
                        theServer->clientList[slot].clientActive = true;
                        sendHelloAck(theServer, slot);
                        sendClientList(theServer, slot);
                        clearMsgData(theServer, slot);
                        clientActive = true;
                }
                else if (clientActive && checkHeaderValid(theServer, slot)) {
                        processHeaderInfo(theServer, slot);
                }
        }
        clearBuffer(theServer, slot);
}


/*****************************************************************************
*                        CLIENT MESSAGE PROCESSING
******************************************************************************/


/*
 * name:      clientReadCall
 * purpose:   does one read call from the client
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   if the client left without warning, remove the client
 */
void clientReadCall(server *theServer, int clientSlot)
{
        int bufferSize = 5000;
        char *readBuffer = malloc(bufferSize);
        checkAllocError(readBuffer);

        int returnVal = read(theServer->clientList[clientSlot].clientSD, 
                        readBuffer, bufferSize);

        if (returnVal == 0 || returnVal == -1) {
                removeClient(theServer, clientSlot);
                return;
        }
        
        theServer->clientList[clientSlot].readBuffer = readBuffer;
        theServer->clientList[clientSlot].bufferRead = 0;
        theServer->clientList[clientSlot].bufferSize = returnVal;
}


/*
 * name:      readClientHeader
 * purpose:   reads the full or parital message header from the read buffer
 * arguments: the server instance, the client slot
 * returns:   number of bytes read from the header (50 if read in full)
 * effects:   populates header, headerRead, bufferRead fields
 */
int readClientHeader(server *theServer, int clientSlot)
{
        clientInfo *theInfo = &theServer->clientList[clientSlot];
        msgHeader *receivedHeader = theServer->clientList[clientSlot].header;
        int headerSize = sizeof(msgHeader);
        int headerBytesLeft = headerSize - theInfo->headerRead;
        int readBufferLeft = theInfo->bufferSize - theInfo->bufferRead;

        // we don't have a full header, so copy what we have
        if (readBufferLeft < headerBytesLeft) {
                memcpy((char *)receivedHeader + theInfo->headerRead, 
                        theInfo->readBuffer + theInfo->bufferRead, 
                        readBufferLeft);
                theInfo->bufferRead = theInfo->bufferSize;
                theInfo->headerRead += readBufferLeft;
                return theInfo->headerRead;
        }

        // we have a full header
        memcpy((char *)receivedHeader + theInfo->headerRead, 
                theInfo->readBuffer + theInfo->bufferRead, 
                headerBytesLeft);
        theInfo->bufferRead += headerBytesLeft;
        theInfo->headerRead = headerSize;

        // convert from network to host byte order
        receivedHeader->msgType = ntohs(receivedHeader->msgType);          
        receivedHeader->contentLength = ntohl(receivedHeader->contentLength);
        receivedHeader->msgID = ntohl(receivedHeader->msgID);

        return headerSize;
}


/*
 * name:      processHeaderInfo
 * purpose:   checks the header message type and processes the header or data
 *            accordingly
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   removes and disconnects the client if it sends EXIT, a second 
 *            HELLO, or has the wrong clientID
 */
void processHeaderInfo(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        if (theInfo.header->msgType == 3) {
                sendClientList(theServer, clientSlot);
                clearMsgData(theServer, clientSlot);
        }
        else if (theInfo.header->msgType == 5) {
                if (!processChatMessage(theServer, clientSlot)) {
                        return;
                }
                clearMsgData(theServer, clientSlot);
        }
        else if (theInfo.header->msgType == 6) {
                removeClient(theServer, clientSlot);
        }
}


/*
 * name:      processChatMessage
 * purpose:   if full message was read, finds the clientSD to forward to and
 *            forwards. If client doesn't exist, sends cannot deliver to sender
 * arguments: the server instance, the client slot
 * returns:   true if full message was read and forwarded / errored, false if
 *            full message was not yet read
 * effects:   none
 */
bool processChatMessage(server *theServer, int clientSlot)
{
        clientInfo *theInfo = &theServer->clientList[clientSlot];
        int contentRead = readClientMessage(theServer, clientSlot);

        // we didn't read all the content, so return to continue reading later
        if (contentRead < theInfo->header->contentLength) {
                return false;
        }

        // find the clientSD to send message to
        int clientSD = -1;
        for (int i = 0; i < theServer->numClients; i++) {
                if (strcmp(theServer->clientList[i].clientID, 
                theInfo->header->msgDest) == 0) {
                        clientSD = theServer->clientList[i].clientSD;
                }
        }

        // the clientSD does not exist or is itself, so can't deliver message
        if (clientSD == theInfo->clientSD) {
                removeClient(theServer, clientSlot);
        }
        else if (clientSD == -1) {
                sendCannotDeliver(theServer, clientSlot);
                removeClient(theServer, clientSlot);
        }
        else {
                forwardClientMessage(theServer, clientSlot, *theInfo->header, 
                        clientSD);
        }
        return true;
}


/*
 * name:      readClientMessage
 * purpose:   reads the full or partial CHAT message content from the read 
 *            buffer
 * arguments: the server instance, the client slot
 * returns:   number of bytes read from the header (contentLength if read 
 *            in full)
 * effects:   populates msgContent, contentRead, bufferRead fields
 */
int readClientMessage(server *theServer, int clientSlot)
{
        clientInfo *theInfo = &theServer->clientList[clientSlot];

        int msgSize = theInfo->header->contentLength;
        int contentBytesLeft = msgSize - theInfo->contentRead;
        int readBufferLeft = theInfo->bufferSize - theInfo->bufferRead;

        // we don't have a full message, so copy what we have
        if (readBufferLeft < contentBytesLeft) {
                memcpy(theInfo->msgContent + theInfo->contentRead, 
                        theInfo->readBuffer + theInfo->bufferRead, 
                        readBufferLeft);
                theInfo->bufferRead = theInfo->bufferSize;
                theInfo->contentRead += readBufferLeft;
                return theInfo->contentRead;
        }
        
        // we have a full message
        memcpy(theInfo->msgContent + theInfo->contentRead, 
                theInfo->readBuffer + theInfo->bufferRead, 
                contentBytesLeft);

        theInfo->bufferRead += contentBytesLeft;
        theInfo->contentRead = msgSize;
        // printf("%s\n", theInfo->msgContent);

        return msgSize;
}



/*****************************************************************************
*                        SERVER TO CLIENT COMMUNICATION
******************************************************************************/

/*
 * name:      sendHelloAck
 * purpose:   sends a HELLO ACK (type 2) message to the client
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   none
 */
void sendHelloAck(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        msgHeader header;
        header.msgType = htons(2);
        strcpy(header.msgSource, "Server");
        header.msgSource[sizeof(header.msgSource) - 1] = '\0';
        strcpy(header.msgDest, theInfo.header->msgSource);
        header.msgDest[sizeof(header.msgDest) - 1] = '\0';
        header.contentLength = htonl(0);
        header.msgID = htonl(0);

        int returnVal = write(theInfo.clientSD, &header, sizeof(msgHeader));
        checkReturnVal(returnVal, theServer, clientSlot);
}


/*
 * name:      sendClientList
 * purpose:   sends a CLIENT LIST (type 4) message to the client
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   none
 */
void sendClientList(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        msgHeader header;
        header.msgType = htons(4);
        strcpy(header.msgSource, "Server");
        header.msgSource[sizeof(header.msgSource) - 1] = '\0';
        strcpy(header.msgDest, theInfo.header->msgSource);
        header.msgDest[sizeof(header.msgDest) - 1] = '\0';
        header.msgID = htonl(0);

        int dataSize = 0;
        char *listBuffer = malloc(450);
        checkAllocError(listBuffer);
        for (int i = 0; i < theServer->numClients; i++) {
                if (theServer->clientList[i].clientActive) {
                        int clientNameLen = 
                                strlen(theServer->clientList[i].clientID) + 1;
                        memcpy(listBuffer + dataSize, 
                        theServer->clientList[i].clientID, clientNameLen);
                        dataSize += clientNameLen;
                }
        }
        header.contentLength = htonl(dataSize);

        int totalSize = sizeof(msgHeader) + dataSize;
        char *writeBuffer = malloc(totalSize);
        checkAllocError(writeBuffer);

        memcpy(writeBuffer, &header, sizeof(msgHeader));
        memcpy(writeBuffer + sizeof(msgHeader), listBuffer, dataSize);
        int returnVal = write(theInfo.clientSD, writeBuffer, totalSize);
        checkReturnVal(returnVal, theServer, clientSlot);

        free(listBuffer);
        free(writeBuffer);
}


/*
 * name:      forwardClientMessage
 * purpose:   sends a CHAT (type 5) message to the recipient client
 * arguments: the server instance, the client slot, the header to forward, 
 *            the socket descriptor of the recipient
 * returns:   none
 * effects:   none
 */
void forwardClientMessage(server *theServer, int clientSlot, 
        msgHeader forwardHeader, int clientSD)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        forwardHeader.msgType = htons(forwardHeader.msgType);          
        forwardHeader.contentLength = htonl(forwardHeader.contentLength);
        forwardHeader.msgID = htonl(forwardHeader.msgID);

        int totalSize = sizeof(msgHeader) + theInfo.header->contentLength;
        char *totalMessage = malloc(totalSize);
        checkAllocError(totalMessage);

        memcpy(totalMessage, &forwardHeader, sizeof(msgHeader));
        memcpy(totalMessage + sizeof(msgHeader), theInfo.msgContent, 
                totalSize - sizeof(msgHeader));

        int returnVal = write(clientSD, totalMessage, totalSize);
        checkReturnVal(returnVal, theServer, clientSlot);
}


/*
 * name:      sendAlreadyPresent
 * purpose:   sends a ERROR_ALREADY_PRESENT (type 7) message to the client
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   none
 */
void sendAlreadyPresent(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        msgHeader header;
        header.msgType = htons(7);
        strcpy(header.msgSource, "Server");
        header.msgSource[sizeof(header.msgSource) - 1] = '\0';
        strcpy(header.msgDest, theInfo.header->msgSource);
        header.msgDest[sizeof(header.msgDest) - 1] = '\0';
        header.contentLength = htonl(0);
        header.msgID = htonl(0);

        int returnVal = write(theInfo.clientSD, &header, sizeof(msgHeader));
        checkReturnVal(returnVal, theServer, clientSlot);
}


/*
 * name:      sendCannotDeliver
 * purpose:   sends a ERROR_CANNOT_DELIVER (type 8) message to the client
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   none
 */
void sendCannotDeliver(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        msgHeader header;
        header.msgType = htons(8);
        strcpy(header.msgSource, "Server");
        header.msgSource[sizeof(header.msgSource) - 1] = '\0';
        strcpy(header.msgDest, theInfo.header->msgSource);
        header.msgDest[sizeof(header.msgDest) - 1] = '\0';
        header.contentLength = htonl(0);
        header.msgID = htonl(theInfo.header->msgID);

        int returnVal = write(theInfo.clientSD, &header, sizeof(msgHeader));
        checkReturnVal(returnVal, theServer, clientSlot);
}



/*****************************************************************************
*                             CHECKING FUNCTIONS
******************************************************************************/


/*
 * name:      checkHeaderValid
 * purpose:   checks the header fields to ensure these are valid 
 * arguments: the server instance, the client slot
 * returns:   true if all fields are valid, false otherwise
 * effects:   none
 */
bool checkHeaderValid(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];
        bool foundMistake = false;
        if (strcmp(theInfo.clientID, theInfo.header->msgSource) != 0) {
                foundMistake = true;
        }
        else if ((theInfo.header->msgSource[sizeof(theInfo.header->msgSource) - 1] != '\0')
        || (theInfo.header->msgDest[sizeof(theInfo.header->msgDest) - 1] != '\0')) {
                foundMistake = true;
        }
        else if ((theInfo.header->msgType == 3) 
        || (theInfo.header->msgType == 6)) {
                if ((strcmp("Server", theInfo.header->msgDest) != 0)
                || (theInfo.header->contentLength != 0)
                || (theInfo.header->msgID != 0)) {
                        foundMistake = true;
                }
        }
        else if (theInfo.header->msgType == 5) {
                if ((theInfo.header->contentLength < 1) 
                || (theInfo.header->contentLength > 400)
                || (theInfo.header->msgID < 1)
                || (strcmp(theInfo.header->msgDest, "Server") == 0)
                || (theInfo.header->msgDest[0] == '\0')
                || (checkOnlyWhiteSpace(theInfo.header->msgDest))) {
                        foundMistake = true;
                }
        }
        else {
                foundMistake = true;
        }
        if (foundMistake) {
                removeClient(theServer, clientSlot);
                return false;
        }

        // printHeader(theInfo.header);
        return true;
}

/*
 * name:      checkHelloMsg
 * purpose:   checks that the message being handled is a valid HELLO message
 * arguments: the server instance, the client slot
 * returns:   none
 * effects:   terminates the client if the message is not valid and removes its
 *            data
 */
bool checkHelloMsg(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        for (int i = 0; i < theServer->numClients; i++) {
                if ((strcmp(theServer->clientList[i].clientID, 
                        theInfo.header->msgSource) == 0)) {
                        sendAlreadyPresent(theServer, clientSlot);
                        removeClient(theServer, clientSlot);
                        return false;
                }
        }

        if (!checkHelloHeader(theServer, clientSlot)) {
                removeClient(theServer, clientSlot);
                return false;
        }
        if (!checkSpaceInClientList(theServer, clientSlot)) {
                removeClient(theServer, clientSlot);
                return false;
        }

        // printHeader(theInfo.header);
        strcpy(theServer->clientList[clientSlot].clientID, 
                theServer->clientList[clientSlot].header->msgSource);
        return true;
}


/*
 * name:      checkHelloHeader
 * purpose:   checks if the HELLO header fields are valid
 * arguments: the server instance, the client slot
 * returns:   true if the fields are valid, false if not
 * effects:   none
 */
bool checkHelloHeader(server *theServer, int clientSlot)
{
        clientInfo theInfo = theServer->clientList[clientSlot];

        if ((theInfo.header->msgType != 1)
        || (theInfo.header->msgSource[sizeof(theInfo.header->msgSource) - 1] != '\0')
        || (theInfo.header->msgDest[sizeof(theInfo.header->msgDest) - 1] != '\0')
        || (strcmp(theInfo.header->msgDest, "Server") != 0)
        || (strcmp(theInfo.header->msgSource, "Server") == 0)
        || (theInfo.header->msgSource[0] == '\0')
        || (checkOnlyWhiteSpace(theInfo.header->msgSource))
        || (theInfo.header->contentLength != 0)
        || (theInfo.header->msgID != 0)) {
                return false;
        }
        return true;
}


/*
 * name:      checkSpaceInClientList
 * purpose:   checks if there is space for a new client in the list
 * arguments: the server instance, the client slot
 * returns:   true if there is space, false if there is not
 * effects:   none
 */
bool checkSpaceInClientList(server *theServer, int clientSlot)
{
        int dataSize = 0;
        for (int i = 0; i < theServer->numClients; i++) {
                if (theServer->clientList[i].clientActive) {
                        int clientNameLen = 
                                strlen(theServer->clientList[i].clientID) + 1;
                        dataSize += clientNameLen;
                }
        }
        dataSize += strlen(theServer->clientList[clientSlot].header->msgSource) + 1;
        if (dataSize > 400) {
                return false;
        }
        return true;
}


/*
 * name:      checkIDExists
 * purpose:   checks if a clientID already exists in the list
 * arguments: the server instance, the client slot
 * returns:   true if the ID exists, false if not
 * effects:   none
 */
bool checkIDExists(server *theServer, int clientSlot) 
{
        for (int i = 0; i < theServer->numClients; i++) {
                if ((strcmp(theServer->clientList[i].clientID, 
                theServer->clientList[clientSlot].header->msgSource) == 0)) {

                        if (i != clientSlot) {
                                return true;
                        }
                }
        }
        return false;
}


bool checkOnlyWhiteSpace(char* String)
{
        int counter = 0;
        bool foundRegChar = false;
        while (String[counter] != '\0') {
                if (String[counter] != ' ') {
                        foundRegChar = true;
                }
                counter++;
        }

        if (foundRegChar) {
                return false;
        }
        return true;
}


/*
 * name:      checkNegOneError
 * purpose:   checks the return value of a function and exits the program if 
 *            it is -1
 * arguments: the return value
 * returns:   none
 * effects:   none
 */
void checkNegOneError(int returnVal)
{
        if (returnVal < 0) {
                printf("Something went wrong, program exiting\n");
                exit(EXIT_FAILURE);
        }
}


/*
 * name:      checkReturnVal
 * purpose:   checks the return value provided and removes the client if it 
 *            is -1
 * arguments: the return value, the server instance, the client slot
 * returns:   none
 * effects:   removes a client if the value is -1
 */
void checkReturnVal(int returnVal, server *theServer, int clientSlot)
{
        if (returnVal == -1) {
                removeClient(theServer, clientSlot);
        }
}


/*
 * name:      checkAllocError
 * purpose:   checks an pointer to allocated memory and exits the program if 
 *            it is NULL
 * arguments: the return value
 * returns:   none
 * effects:   none
 */
void checkAllocError(char *memAllocated)
{
        if (memAllocated == NULL) {
                printf("Memory could not be allocated, program exiting\n");
                exit(EXIT_FAILURE);
        }
}


/*****************************************************************************
*                         CLIENT LIST ORGANIZATION
******************************************************************************/


/*
 * name:      reorderClientList
 * purpose:   reorders the clientList in the case that a HELLO message arrived 
 *            in multiple parts
 * arguments: the server instance, the client slot of the HELLO message
 * returns:   the (new) clientSlot
 * effects:   reorders the clientList
 */
int reorderClientList(server *theServer, int clientSlot)
{
        if (clientSlot != (theServer->numClients - 1)) {
                clientInfo *theInfo1 = &theServer->clientList[clientSlot];
                clientInfo *theInfo2 = malloc(sizeof(clientInfo));
                checkAllocError((char *)theInfo2);
                makeClientInfoCopy1(theInfo1, theInfo2);

                clearListSlot(theServer, clientSlot);
                shiftClients(theServer, clientSlot);

                clientInfo *theInfo3 = &theServer->clientList[theServer->numClients];
                makeClientInfoCopy2(theInfo2, theInfo3);
                theServer->numClients++;
                return theServer->numClients - 1;
        }

        return clientSlot;
}


/*
 * name:      shiftClients
 * purpose:   shifts clients over when a client was removed in the middle of 
 *            the clientList
 * arguments: the server instance, the empty spot in the list
 * returns:   none
 * effects:   reorganizes the clientList
 */
void shiftClients(server *theServer, int moveSpot)
{
        for (int i = moveSpot; i < theServer->numClients; i++) {
                if (i == (theServer->numClients - 1)) {
                        clearListSlot(theServer, i);
                        break;
                }
                clientInfo *theInfo1 = &theServer->clientList[i];
                clientInfo *theInfo2 = &theServer->clientList[i + 1];

                makeClientInfoCopy2(theInfo2, theInfo1);
        }

        if (theServer->numClients != 0) {
                theServer->numClients--;
        }
}



/*
 * name:      makeClientInfoCopy1
 * purpose:   makes a deep copy from one clientInfo struct to another where 
 *            memory was not previously allocated
 * arguments: the from clientInfo struct, and the to clientInfo struct
 * returns:   none
 * effects:   none
 */
void makeClientInfoCopy1(clientInfo *fromClient, clientInfo *toClient)
{
        toClient->clientSD = fromClient->clientSD;
        toClient->clientID = malloc(20);
        checkAllocError(toClient->clientID);
        memcpy(toClient->clientID, fromClient->clientID, 20);

        toClient->bufferSize = fromClient->bufferSize;
        toClient->bufferRead = fromClient->bufferRead;
        toClient->readBuffer = malloc(fromClient->bufferSize);
        checkAllocError(toClient->readBuffer);
        memcpy(toClient->readBuffer, fromClient->readBuffer, fromClient->bufferSize);

        toClient->headerRead = fromClient->headerRead;
        toClient->header = malloc(50);
        checkAllocError((char *)toClient->header);
        memcpy(toClient->header, fromClient->header, 50);

        toClient->contentRead = fromClient->contentRead;
        toClient->msgContent = malloc(400);
        checkAllocError(toClient->msgContent);
        memcpy(toClient->msgContent, fromClient->msgContent, 400);

        toClient->clientActive = fromClient->clientActive;
        toClient->timeAdded = fromClient->timeAdded;
}


/*
 * name:      makeClientInfoCopy2
 * purpose:   makes a deep copy from one clientInfo struct to another where 
 *            memory was previously allocated in both
 * arguments: the from clientInfo struct, and the to clientInfo struct
 * returns:   none
 * effects:   none
 */
void makeClientInfoCopy2(clientInfo *fromClient, clientInfo *toClient)
{
        toClient->clientSD = fromClient->clientSD;
        memcpy(toClient->clientID, fromClient->clientID, 20);

        toClient->bufferSize = fromClient->bufferSize;
        toClient->bufferRead = fromClient->bufferRead;
        toClient->readBuffer = malloc(fromClient->bufferSize);
        checkAllocError(toClient->readBuffer);
        memcpy(toClient->readBuffer, fromClient->readBuffer, fromClient->bufferSize);
        free(fromClient->readBuffer);
        fromClient->readBuffer = NULL;

        toClient->headerRead = fromClient->headerRead;
        memcpy(toClient->header, fromClient->header, 50);

        toClient->contentRead = fromClient->contentRead;
        memcpy(toClient->msgContent, fromClient->msgContent, 400);

        toClient->clientActive = fromClient->clientActive;
        toClient->timeAdded = fromClient->timeAdded;
}



/*****************************************************************************
*                          REMOVE / RESET FUNCTIONS
******************************************************************************/

/*
 * name:      removeClient
 * purpose:   removes a client from the clientList
 * arguments: the server instance, the client list slot
 * returns:   none
 * effects:   removes the client's associated data, closes the connection 
 *            with the client, and reorders the clientList
 */
void removeClient(server *theServer, int clientSlot)
{
        close(theServer->clientList[clientSlot].clientSD);
        FD_CLR(theServer->clientList[clientSlot].clientSD, 
                &theServer->activeFDSet);
        clearListSlot(theServer, clientSlot);
        shiftClients(theServer, clientSlot);
}


/*
 * name:      removeExpiredClients
 * purpose:   removes any clients that have timedout
 * arguments: the server instance
 * returns:   true if there was an expired client, false if not
 * effects:   removes expired clients and its associated data
 */
bool removeExpiredClients(server *theServer)
{
        unsigned long long currTime = getCurrTime(theServer);
        unsigned long long oneMin = 60 * 1000000LL;
        bool foundExpired = false;
        for (int i = 0; i < theServer->numClients; i++) {
                if (theServer->clientList[i].timeAdded == -1) {
                        continue;
                }
                if (currTime >= (theServer->clientList[i].timeAdded + oneMin)) {
                        removeClient(theServer, i);
                        foundExpired = true;
                }
        }

        if (foundExpired) {
                return true;
        }
        return false;
}


/*
 * name:      clearListSlot
 * purpose:   clears any client data, so can be used when client is removed
 *            from server record
 * arguments: the server instance, the client list slot
 * returns:   none
 * effects:   removes all client data
 */
void clearListSlot(server *theServer, int clientSlot)
{
        theServer->clientList[clientSlot].clientSD = -1;
        memset(theServer->clientList[clientSlot].clientID, 0, 20);

        theServer->clientList[clientSlot].bufferSize = 0;
        theServer->clientList[clientSlot].bufferRead = 0;
        if (theServer->clientList[clientSlot].readBuffer != NULL) {
                free(theServer->clientList[clientSlot].readBuffer);
                theServer->clientList[clientSlot].readBuffer = NULL;
        }
        
        theServer->clientList[clientSlot].headerRead = 0;
        memset(theServer->clientList[clientSlot].header, 0, sizeof(msgHeader));

        theServer->clientList[clientSlot].contentRead = 0;
        memset(theServer->clientList[clientSlot].msgContent, 0, 400);

        theServer->clientList[clientSlot].clientActive = false;
        theServer->clientList[clientSlot].timeAdded = -1;
}

/*
 * name:      clearMsgData
 * purpose:   clears the header and content info for a client so new data can
 *            be added
 * arguments: the server instance, the client list slot
 * returns:   none
 * effects:   removes content and header data
 */
void clearMsgData(server *theServer, int clientSlot)
{
        strcpy(theServer->clientList[clientSlot].clientID, 
                theServer->clientList[clientSlot].header->msgSource);
        
        theServer->clientList[clientSlot].headerRead = 0;
        memset(theServer->clientList[clientSlot].header, 0, sizeof(msgHeader));

        theServer->clientList[clientSlot].contentRead = 0;
        memset(theServer->clientList[clientSlot].msgContent, 0, 400);

        theServer->clientList[clientSlot].timeAdded = -1;
}


/*
 * name:      clearBuffer
 * purpose:   clears the bufferSize, bufferRead, and the allocated buffer for 
 *            a client
 * arguments: the server instance, the client list slot
 * returns:   none
 * effects:   frees the buffer memory
 */
void clearBuffer(server *theServer, int clientSlot)
{
        theServer->clientList[clientSlot].bufferSize = 0;
        theServer->clientList[clientSlot].bufferRead = 0;
        if (theServer->clientList[clientSlot].readBuffer != NULL) {
                free(theServer->clientList[clientSlot].readBuffer);
                theServer->clientList[clientSlot].readBuffer = NULL;
        }
}



/*****************************************************************************
*                               HELPER FUNCTIONS
******************************************************************************/

/*
 * name:      createSocket
 * purpose:   sets up the server socket connection
 * arguments: the server instance
 * returns:   none
 * effects:   populates the listenSD field of the server instance
 */
void createSocket(server *theServer)
{
        //socket setup
        struct sockaddr_in serverAddress;
        socklen_t serverAddressLength;
        int listenSD = socket(AF_INET, SOCK_STREAM, 0);
        checkNegOneError(listenSD);
        theServer->listenSD = listenSD;

        //ensure to close the socket and port after termination
        int opt = 1;
        int returnVal = setsockopt(listenSD, SOL_SOCKET, SO_REUSEADDR, &opt, 
                        sizeof(opt));
        checkNegOneError(returnVal);

        //setup to bind the socket to port specified port and any IP address
        //this is the setup specifying the Proxy info
        memset(&serverAddress, 0, sizeof(struct sockaddr_in));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(theServer->portNumber);
        returnVal = bind(listenSD, (struct sockaddr *)&serverAddress, 
                        sizeof(struct sockaddr_in));
        checkNegOneError(returnVal);
}


/*
 * name:      getTimeoutValue
 * purpose:   gets the timeout value of the client with the shortest time to 
 *            timeout
 * arguments: the server instance
 * returns:   the timeout value in seconds
 * effects:   none
 */
long long getTimeoutValue(server *theServer)
{
        long long lowestTimeLeft = LLONG_MAX;
        unsigned long long currTime = getCurrTime(theServer);
        unsigned long long oneMinute = 60 * 1000000LL;

        for (int i = 0; i < theServer->numClients; i++) {
                if (theServer->clientList[i].timeAdded != -1) {
                        int timeLeft = theServer->clientList[i].timeAdded 
                                        + oneMinute - currTime;
                        if ((timeLeft >= 0) && (timeLeft < lowestTimeLeft)) { 
                                lowestTimeLeft = timeLeft;
                        }
                }
        }
        if (lowestTimeLeft == LLONG_MAX) {
                return 100000000;
        }
        return lowestTimeLeft;
}


/*
 * name:      getInitialTime
 * purpose:   gets the initial time at program startup
 * arguments: none
 * returns:   the initial time
 * effects:   none
 */
unsigned long long getInitialTime()
{
        struct timespec currTime;
        int returnVal = clock_gettime(CLOCK_MONOTONIC, &currTime);
        checkNegOneError(returnVal);

        unsigned long long theTime = 
                (currTime.tv_sec * 1000000LL) + (currTime.tv_nsec / 1000);

        return theTime;
}


/*
 * name:      getCurrTime
 * purpose:   gets the current time minus the initial time to leave enough bits
 * arguments: none
 * returns:   the current time
 * effects:   none
 */
unsigned long long getCurrTime(server *theServer)
{
        struct timespec currTime;
        int returnVal = clock_gettime(CLOCK_MONOTONIC, &currTime);
        checkNegOneError(returnVal);

        unsigned long long timeMicroSec = 
                (currTime.tv_sec * 1000000LL) + (currTime.tv_nsec / 1000);
        unsigned long long theTime = timeMicroSec - theServer->initServerTime;

        return theTime;
}



/*
 * name:      printHeader
 * purpose:   prints out message header information
 * arguments: a pointer to the header
 * returns:   none
 * effects:   none
 */
// void printHeader(msgHeader *header) 
// {
//         printf("\n");
//         if (header->msgType == 1) {
//                 printf("HELLO\n");
//         }
//         else if (header->msgType == 3) {
//                 printf("CLIENT LIST REQ\n");
//         }
//         else if (header->msgType == 5) {
//                 printf("CHAT\n");
//         }
//         else if (header->msgType == 6) {
//                 printf("EXIT\n");
//         }
//         printf("Message Header:\n");
//         printf("  msgType: %u\n", header->msgType);
//         printf("  msgSource: %s\n", header->msgSource);
//         printf("  msgDest: %s\n", header->msgDest);
//         printf("  contentLength: %u\n", header->contentLength);
//         printf("  msgID: %u\n", header->msgID);
//         printf("\n");
// }


/*
 * name:      printClientList
 * purpose:   prints out the client list
 * arguments: the server instance
 * returns:   none
 * effects:   none
 */
// void printClientList(server *theServer)
// {
//         for (int i = 0; i < theServer->numClients; i++) {
//                 if (theServer->clientList[i].clientActive) {
//                         int clientNameLen = strlen(theServer->clientList[i].header->msgSource) + 1;
//                         printf("Client: %s\n", theServer->clientList[i].header->msgSource);
//                 }
//         }
//         printf("\n");
// }