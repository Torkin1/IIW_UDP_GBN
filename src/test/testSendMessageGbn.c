#include "gbn/gbn.h"
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

// tested with MTU = 20 and winsize = 10
// see if packets are sent correctly using wireshark on loopback interface

long int msg = "ciao a tutti!";

void testSendMessageGbn(){

    int sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in *sendAddr = calloc(1, sizeof(struct sockaddr_in));
    sendAddr ->sin_family = AF_INET;
    sendAddr ->sin_port = 1025;
    sendAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sendMessageGbn(sendSocket, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in), (void *) msg, strlen(msg), NULL);

    pause();



}
