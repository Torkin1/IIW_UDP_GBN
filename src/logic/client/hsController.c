#include "client/hsController.h"
#include "dm_protocol/dm_protocol.h"
#include "logger/logger.h"
#include <string.h>	


int doHandShake(int sd, struct sockaddr *serverAddr, socklen_t addrlen, in_port_t *portBuf){

	Message *hsRequest, *hsResponse;
		
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

	if (hsResponse ->message_header ->status != HS_OK){
		logMsg(E, "doHandShake: server responded with error %d: %s\n", &(hsResponse ->message_header ->status), hsResponse ->payload);
		destroyMessage(hsResponse);
		return -1;
	}

	memcpy(portBuf, hsResponse ->payload, sizeof(in_port_t));

	return 0;
}
