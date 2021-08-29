#include "gbn/packet.h"
#include "testSuites.h"
#include <string.h>
#include "logger/logger.h"
#include <stdio.h>

static Packet *p1, *p2;
static char* msg = "hello there!";
static uint8_t *buf;
static char errorMsg[1024];

void cleanup(){

    destroyPacket(p1);
    destroyPacket(p2);
    free(buf);

}

void testSerialize(){

    logMsg(D, "testSerialize: sizeof in_port_t is: %d\n", sizeof(in_port_t));
    p1 = newPacket();
    p1 ->data = calloc(strlen(msg) + 1, sizeof(char));
    memcpy(p1 -> data, msg, strlen(msg) + 1);
    p1->header->dataLen = strlen(msg) + 1;
    p1 ->header ->isAck = true;
    p1 ->header ->endIndex = 20;
    p1 ->header ->index = 3;
    p1 ->header ->msgId = 31;
    p1 ->header ->ackPort = 255;

    buf = serializePacket(p1);
    p2 = deserializePacket(buf);

    sprintf(errorMsg, "testSerialize: expected: %d, actual: %d\n", p1 ->header ->ackPort, p2 ->header ->ackPort);

    assertEquals(
                 &(p1 ->header ->ackPort),
                 &(p2 ->header ->ackPort),
                 sizeof(in_port_t),
                 errorMsg,
                 cleanup
                 );
    cleanup();
}



