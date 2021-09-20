#include <string.h>	
#include <unistd.h>
#include <stdlib.h>
#include "logger/logger.h"
#include "dm_protocol/dm_protocol.h"
#include <netinet/ip.h>
#include "dataStructures/hTable.h"
#include <errno.h>
#include "client/controller.h"

// holds all names of command currently accepted by the client as its first arg. When adding a new command, remember to update initClient()
//static char *commandNames[COMMANDS_NUM];
static HashTable *supportedCommandNames;

// default address of server
struct sockaddr_in *serverAddr;

// socket used by client to send data;
static int sd = -1;

/**
    Creates a dynamically allocated hashtable and populates it with op codes using their names as keys, if not created before.
    @return Command code corresponding to given name. Returns NULL if no command is registered with such name
*/
DmProtocol_command *getCommandByName(char *name){

    if (supportedCommandNames == NULL){
        supportedCommandNames = newHashTable();
        
        // TODO: populate table with commands using names as key
        char *putKey = calloc(strlen("PUT") + 1, sizeof(char));
        strcpy(putKey, "PUT");
        DmProtocol_command *putCommand = calloc(1, sizeof(DmProtocol_command));
        *putCommand = PUT;
        addToHashTable(supportedCommandNames, putKey, strlen(putKey), putCommand);

        char *getKey = calloc(strlen("GET") + 1, sizeof(char));
        strcpy(getKey, "GET");
        DmProtocol_command *getCommand = calloc(1, sizeof(DmProtocol_command));
        *getCommand = GET;
        addToHashTable(supportedCommandNames, getKey, strlen(getKey), getCommand);

        char *listKey = calloc(strlen("LIST") + 1, sizeof(char));
        strcpy(listKey, "LIST");
        DmProtocol_command *listCommand = calloc(1, sizeof(DmProtocol_command));
        *listCommand = LIST;
        addToHashTable(supportedCommandNames, listKey, strlen(listKey), listCommand);
    }

    return (DmProtocol_command *) getValueFromHashTable(supportedCommandNames, name, strlen(name));
}

/**
    Creates socket usable by all client components if not created before, then it returns it.
    Socket shall be created conforming to the protocol used (i.e. using dmProtocol implies creating a UDP internet socket)
    Socket can be closed using close() as usual.
    @return socket descriptor if successful, else -1
*/
int getGlobalSocket(){

    if (sd < 0){
        sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sd < 0){
            int err = errno;
            logMsg(E, "getGlobalSocket: unable to create global socket: %s\n", strerror(err));
            return -1;
        }
    }
    return sd;
}

/**
	dynamically allocates a struct holding server default address, if it has not been created before.
	@return default server address.
*/
struct sockaddr *getServerAddress(){

    if (serverAddr == NULL){
        serverAddr = calloc(1, sizeof(struct sockaddr_in));
        serverAddr ->sin_family = AF_INET;
        serverAddr ->sin_addr.s_addr = htonl(DEFAULT_SERVER_ADDR);
        serverAddr ->sin_port = htons(DEFAULT_SERVER_PORT);
    }
    return (struct sockaddr *) serverAddr;
}

int parseCommandName(int argc, char *argv[], DmProtocol_command *toInvoke){

    char *toInvokeName;
	
	// parses command name
	if (argc < 1){
		logMsg(E, "%s: not enough arguments\n", argv[0]);
		logMsg(I, "%s: usage: ./client.c command args ...\n", argv[0]);
		return -1;
	}
	toInvokeName = argv[1];

	// retrieves command corresponding with given command name if there is one, else an error is reported and exits
	DmProtocol_command *toInvokeBuf = getCommandByName(toInvokeName);
	if (toInvokeBuf == NULL){
		logMsg(E, "%s: unrecognized operation: %s\n", argv[0], toInvokeName);
		return -1;
	}
	*toInvoke = *toInvokeBuf;
	return 0;
}

int doCommand(DmProtocol_command toInvoke, char *toInvokeArgs[]){

	switch (toInvoke){
		
		case PUT: 
            
			// TODO: return invokePut
			break;

        case GET:
            
			// TODO: return invokeGet
			break;

        case LIST:
            
			// TODO: return invokeList
			break;

        default:
		logMsg(E, "%s: unsupported operation with code: %d\n", toInvoke);
		return -1;
    }

	return 0;	
}
