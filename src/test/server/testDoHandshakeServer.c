#include "client/hsController.h"
#include "dm_protocol/dm_protocol.h"
#include "../testSuites.h"
#include <string.h>
#include <unistd.h>
#include "logger/logger.h"
#include "arpa/inet.h"
#define EXPECTED_PORT 50321


void testDoHandshakeServer(){

    int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    in_port_t expected, actual = -1;
    expected = EXPECTED_PORT;

        // server

    bind(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    logMsg(D, "listening on port %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    struct sockaddr clientAddr;
    socklen_t clientAddrLen;
    Message *request, *response;

    receiveMessageDMProtocol(sd, &clientAddr, &clientAddrLen, &request);
    logMsg(I, "testDoHandShake: Server; received handshake request\n");

    if (request->message_header->command != HS)
    {
        logMsg(E, "testDoHandshake: server: command is %d instead of %d\n", request->message_header->command, HS);
        exit(-1);
    }

    response = newMessage();
    response->payload = &expected;
    response->message_header->command = HS;
    response->message_header->status = HS_OK;
    response->message_header->payload_lentgh = sizeof(in_port_t);

    sendMessageDMProtocol(sd, &clientAddr, clientAddrLen, response);
    logMsg(I, "testDoHandShake: Server: requested send of handshake response\n");

    pause();



}