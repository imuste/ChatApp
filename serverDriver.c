/*****************************************************************************
 *
 *      serverDriver.c
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

#include "server.h"




/*
 * name:      main
 * purpose:   opens the commands file and initializes a new cache instance
 * arguments: argc, argv
 * returns:   exit success
 * effects:   none
 */
int main(int argc, char *argv[])
{       
        if (argc < 2) {
                fprintf(stderr, "usage: ./a.out <portNumber>\n");
                return EXIT_FAILURE;
        }

        int port = atoi(argv[1]);
        server *theServer = newServer(port);
        
        runChatApp(theServer);

        return EXIT_SUCCESS;
}




