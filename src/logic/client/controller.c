#include <string.h>	
#include <unistd.h>
#include <stdlib.h>
#include "logger/logger.h"
#include <netinet/ip.h>
#include "dataStructures/hTable.h"
#include <errno.h>
#include "client/controller.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

// holds all names of command currently accepted by the client as its first arg. When adding a new command, remember to update initClient()
static HashTable *supportedCommandNames;

// default address of server
struct sockaddr_in *serverAddr;

// socket used by client to send data;
static int globalSd = -1;

/**
    Creates a dynamically allocated hashtable and populates it with op codes using their names as keys, if not created before.
    @return Command code corresponding to given name. Returns NULL if no command is registered with such name
*/
DmProtocol_command *getCommandByName(char *name){

    if (supportedCommandNames == NULL){
        supportedCommandNames = newHashTable();
        
        // populate table with commands using names as key
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
	dynamically allocates a struct holding server default address, if it has not been created before.
	@return default server address.
*/
struct sockaddr_in *getServerAddress(){

    if (serverAddr == NULL){
        serverAddr = calloc(1, sizeof(struct sockaddr_in));
        serverAddr ->sin_family = AF_INET;
        serverAddr ->sin_addr.s_addr = htonl(DEFAULT_SERVER_ADDR);
        serverAddr ->sin_port = htons(DEFAULT_SERVER_PORT);
    }
    return serverAddr;
}



/**
    Creates a UDP socket bounded with client address if not created before, then it returns it.
    Client address is created before bounding. 
    Socket can be closed using close() as usual. 
    @return socket descriptor if successful, else -1
*/
int getGlobalSocket(){

    struct sockaddr_in clientAddr = {0};
    const in_port_t MIN_PORT = 1024, MAX_PORT = 65535;
    in_port_t clientPort;
    int bindRes, bindResErrno;

    if (globalSd < 0)
    {
        globalSd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (globalSd < 0)
        {
            int err = errno;
            logMsg(E, "getGlobalSocket: unable to create global socket: %s\n", strerror(err));
            return -1;
        }

        // tries all available ports until it founds one available
        clientAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        clientAddr.sin_family = AF_INET;
        for (clientPort = MIN_PORT; clientPort <= MAX_PORT; clientPort++)
        {
            clientAddr.sin_port = htons(clientPort);
            bindRes = bind(globalSd, &clientAddr, sizeof(clientAddr));
            bindResErrno = errno;
            if (bindRes == 0)
            {
                logMsg(D, "getGlobalSocket: client port is %d\n", clientPort);
                break;
            }
            else
            {
                if (bindResErrno != EADDRINUSE && bindResErrno != EADDRNOTAVAIL)
                {
                    logMsg(E, "getGlobalSocket: there is no port available to bind to global socket\n");
                    return -1;
                }
            }
        }
    }

    return globalSd;
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

/**
	Implements HS command of dm_protocol.
	@param newSd pointer to buffer which will hold the address where the caller has to send its requests, using the port number sent by the server. Caller must use this socket to only send requests different from handshakes
	@return 0 if a positive response is received, meaning that the server succesfully allocated resources to handle client request and opened a port for it, else -1
*/
int doHandShake(int sd, struct sockaddr *serverAddr, socklen_t addrlen, struct sockaddr_in *handlerAddr){

    Message *hsRequest, *hsResponse;
    in_port_t assignedPort;
		
	// creates handshake request and sends it
	hsRequest = newMessage();
	hsRequest ->message_header ->command = HS;
	sendMessageDMProtocol(sd, serverAddr, addrlen, hsRequest);
	destroyMessage(hsRequest);

	// listens for response. If handshake is successful, we return the port number to the caller, else we return an error
	do
	{
		if (receiveMessageDMProtocol(sd, NULL, NULL, &hsResponse) != 0)
		{
			logMsg(E, "doHandShake: failed to receive message\n");
			return -1;
		}
	} while (hsResponse->message_header->command != HS);	// keeps listening until an handshake response arrives

	if (hsResponse ->message_header ->status != OP_STATUS_OK){
		logMsg(E, "doHandShake: server responded with error %d: %s\n", &(hsResponse ->message_header ->status), hsResponse ->payload);
		destroyMessage(hsResponse);
		return -1;
	}

    memcpy(&assignedPort, hsResponse ->payload, sizeof(in_port_t));
    logMsg(D, "doHandshake: assigned port is %d\n", assignedPort);
    memcpy(handlerAddr, getServerAddress(), sizeof(struct sockaddr_in));
    handlerAddr ->sin_port = htons(assignedPort);
    destroyMessage(hsResponse);

	return 0;
}

/**
	Implements LIST command of dm_protocol.
    Will listen for response until a legal LIST response is received
*/
int doList(){

    int sd = getGlobalSocket();
    struct sockaddr_in assignedServerAddress;
    Message *request, *response;
    const char *FILENAME_DEL = "\n";
        
    // does handshake
    if (doHandShake(sd, (struct sockaddr*) getServerAddress(), sizeof(*getServerAddress()), &assignedServerAddress) != 0){
        logMsg(E, "doList: unable to perform handshake with server\n");
        return -1;
    }
    
    logMsg(D, "doList: handshake performed. Assigned address is %s %d\n", inet_ntoa(assignedServerAddress.sin_addr), ntohs(assignedServerAddress.sin_port));

    // asks server to perform LIST operation and waits for it to send a response
    request = newMessage();
    request ->message_header ->command = LIST;
    request ->payload = FILENAME_DEL;
    request ->message_header ->payload_lentgh = strlen(FILENAME_DEL) + 1;
    if (sendMessageDMProtocol(sd, (struct sockaddr *) &assignedServerAddress, sizeof(assignedServerAddress), request) != 0){
        logMsg(E, "doList: unable to send request to server\n");
        return -1;
    }
    while (receiveMessageDMProtocol(sd, NULL, NULL, &response) != 0 || response ->message_header ->command != LIST){
        logMsg(E, "doList: an error occurred while listening for server response, or received a response for a different command than expected. Still listening ...\n");
    }
    logMsg(D, "doList: request received\n");

    // prints filelist if OK, else handles the error
    if (response -> message_header ->status != OP_STATUS_OK){
        logMsg(E, "doList: server reponded with error %d: %s\n", response ->message_header ->status, response ->payload);
        return -1;
    }
    logMsg(I, "files currently available in server:\n%s\n", response ->payload);

    return 0;
}

int doGet(char *fileName){

    Message *request, *response;
    int fd;
    struct sockaddr_in assignedServerAddress = {0};
    
    if (fileName == NULL){
        logMsg(E, "doGet: fileName can't be NULL\n");
        return -1;
    }

    // does handshake
    if (doHandShake(getGlobalSocket(), (struct sockaddr*) getServerAddress(), sizeof(*getServerAddress()), &assignedServerAddress) != 0){
        logMsg(E, "doGet: unable to perform handshake with server\n");
        return -1;
    }
    
    // requests get to server
    request = newMessage();
    request ->message_header ->command = GET;
    request ->message_header ->payload_lentgh = strlen(fileName) + 1;
    request ->payload = fileName;
    if (sendMessageDMProtocol(getGlobalSocket(), &assignedServerAddress, sizeof(assignedServerAddress), request)){
        logMsg(E, "doGet: failed to send request\n");
        return -1;
    }

    while (receiveMessageDMProtocol(getGlobalSocket(), NULL, NULL, &response) != 0 || response ->message_header ->command != GET){
        logMsg(E, "doGet: an error occurred while listening for server response, or received a response for a different command than expected. Still listening ...\n");
    }

    // if success we open the file and start receiving content from server, else we return an error
    if(response ->message_header ->status != OP_STATUS_OK){
        
        logMsg(E, "doGet: server reponded with error %d: %s\n", response ->message_header ->status, response ->payload);
        return -1;
    }

    fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (receiveFileDMProtocol(getGlobalSocket(), NULL, NULL, fd)){
        logMsg(E, "doGet: failed to receive file conten from server\n");
        return -1;
    }
    logMsg(I, "File %s successfully received\n", fileName);

    return 0;
}

int doPut(char *fileName){

    struct sockaddr_in handlerAddr;
    int fd, err;
    Message *request, *response;
    
    // name must be not NULL
    if (fileName == NULL){
        logMsg(E, "doPut: fileName can't be NULL\n");
        return -1;
    }

    // does handshake
    if (doHandShake(getGlobalSocket(), (struct sockaddr *) getServerAddress(), sizeof(*getServerAddress()), &handlerAddr)){
        logMsg(E, "doPut: handshake failed\n");
        return -1;
    }

    // opens file to hold contents sent by server. If already exists, it is overwritten
    if ((fd = open(fileName, O_RDONLY, S_IRUSR | S_IWUSR)) < 0)
    {
        err = errno;
        logMsg(E, "doPut: failed to open file %s: %s\n", fileName, strerror(err));
        return -1;
    }

    // requests PUT to assigned address and listens for response
    request = newMessage();
    request ->message_header ->command = PUT;
    request ->message_header ->payload_lentgh = strlen(fileName) + 1;
    request ->payload = fileName;
    if (sendMessageDMProtocol(getGlobalSocket(), (struct sockaddr *) &handlerAddr, sizeof(handlerAddr), request)){
        logMsg(E, "doPut: failed to send message\n");
        destroyMessage(request);
        close(fd);
        return -1;
    }
    destroyMessage(request);
    while (receiveMessageDMProtocol(getGlobalSocket(), NULL, NULL, &response) || response->message_header->command != PUT)
    {
        logMsg(E, "doPut: an error occurred while listening for server response, or received a response for a different command than expected. Still listening ...\n");
    }
    
    // if PUT response was successful, we send the file
    if (response->message_header->status != OP_STATUS_OK)
    {
        logMsg(E, "doPut: server reponded with error %d: %s\n", response->message_header->status, response->payload);
        destroyMessage(response);
        close(fd);
        return -1;
    }
    destroyMessage(response);
    if (sendFileDMProtocol(getGlobalSocket(), (struct sockaddr *) &handlerAddr, sizeof(handlerAddr), fd) < 0){
        logMsg(E, "doPut: failed to send file\n");
        close(fd);
        return -1;
    }

    logMsg(I, "File successfully sent to server\n");

    close(fd);
    return 0;
}

int doCommand(DmProtocol_command toInvoke, char *toInvokeArgs[]){

	switch (toInvoke){
		
		case PUT: 
            
			return doPut(toInvokeArgs[0]);
            break;

        case GET:
            
			return doGet(toInvokeArgs[0]);
			break;

        case LIST:
            
            return doList();
			break;

        default:
		logMsg(E, "%s: doCommand: unsupported operation with code: %d\n", toInvoke);
		return -1;
    }

	return 0;	
}
