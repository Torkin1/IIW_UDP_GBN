#include "gbn/gbn.h"
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "gbn/launcher.h"
#include "gbn/launchBattery.h"
#include "gbn/packet.h"
#include "logger/logger.h"
#include <stdio.h>
#include <stdbool.h>
#include "testSuites.h"

// see if packets are sent correctly using wireshark on loopback interface

static char *msg = "ciao a tutti!";
static void *rcvd;
int rcvdSize;

void testRecvMessageGbn(){

    pthread_t launcherTid;
    
    int sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in *sendAddr = calloc(1, sizeof(struct sockaddr_in));
    sendAddr ->sin_family = AF_INET;
    sendAddr ->sin_port = htons(32772);
    sendAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int receiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in *recvAddr = calloc(1, sizeof(struct sockaddr_in));
    recvAddr ->sin_family = AF_INET;
    recvAddr ->sin_port = htons(32772);
    recvAddr ->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(receiveSocket, (struct sockaddr*) recvAddr, sizeof(struct sockaddr_in));

    sendMessageGbn(sendSocket, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in), (void *) msg, strlen(msg) + 1, NULL);
    recvMessageGbn(receiveSocket, NULL, NULL, &rcvd, &rcvdSize);

    assertEquals(msg, rcvd, rcvdSize, NULL, NULL);

    while (sendMessageGbn(sendSocket, (struct sockaddr *) sendAddr, sizeof(struct sockaddr_in), (void *) msg, strlen(msg) + 1, NULL) < 0);
    recvMessageGbn(receiveSocket, NULL, NULL, &rcvd, &rcvdSize);

    assertEquals(msg, rcvd, rcvdSize, NULL, NULL);

    logMsg(D, "testReceiveMessageGbn: recvMessageGbn returned for the last time\n");
    for(int i = 5; i > 0; i --){
        logMsg(D, "testReceiveMessageGbn: closing test in %d secs\n", i);
        sleep(1);
    }

    getLauncherId(&launcherTid);
    notifyLauncher(LAUNCHER_EVENT_SHUTDOWN);
    pthread_join(launcherTid, NULL);


    free(sendAddr);
    free(recvAddr);


}
