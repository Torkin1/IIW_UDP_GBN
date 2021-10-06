#include "server/controller.h"
#include "dm_protocol/dm_protocol.h"
#include "logger/logger.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/ip.h>

struct sockaddr_in *serverAddr;


struct sockaddr_in *getServerAddress(){

    if (serverAddr == NULL){
        serverAddr = calloc(1, sizeof(struct sockaddr_in));
        serverAddr ->sin_family = AF_INET;
        serverAddr ->sin_addr.s_addr = htonl(DEFAULT_SERVER_ADDR);
        serverAddr ->sin_port = htons(DEFAULT_SERVER_PORT);
    }
    return serverAddr;
}

void testHandShake(){
    logMsg(D, "Starting test...\n");

    start_server();

    logMsg(D, "ServerStarted\n");


    struct sockaddr_in clientAddr = {0};
    struct sockaddr_in serverAddr = {0};
    in_port_t clientPort = 8999;
    int clientSd;
    const in_port_t MIN_PORT = 1024, MAX_PORT = 65535;
    int bindRes, bindResErrno;
    Message *hsRequest, *hsResponse;
    in_port_t assignedPort;

    socklen_t addrlen = sizeof(*getServerAddress());



    if((clientSd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0){
        int err = errno;
            logMsg(E, "testHandShake: unable to create clients socket: %s\n", strerror(err));
            return -1;
    }

    // tries all available ports until it founds one available
        clientAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        clientAddr.sin_family = AF_INET;
        for (clientPort = MIN_PORT; clientPort <= MAX_PORT; clientPort++)
        {
            clientAddr.sin_port = htons(clientPort);
            bindRes = bind(clientSd, &clientAddr, sizeof(clientAddr));
            bindResErrno = errno;
            if (bindRes == 0)
            {
                logMsg(D, "Socket on the testHandhake: client port is %d\n", clientPort);
                break;
            }
            else
            {
                if (bindResErrno != EADDRINUSE && bindResErrno != EADDRNOTAVAIL)
                {
                    logMsg(E, "testHandShake: there is no port available to bind to global socket\n");
                    return -1;
                }
            }
        }

        hsRequest = newMessage();
        hsRequest ->message_header ->command = HS;
        sendMessageDMProtocol(clientSd, getServerAddress(), addrlen, hsRequest);
        destroyMessage(hsRequest);

        do
	{
		if (receiveMessageDMProtocol(clientSd, NULL, NULL, &hsResponse) != 0)
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
    getServerAddress()->sin_port = assignedPort;;
    destroyMessage(hsResponse);

    //Message *commandRequest;

    
}