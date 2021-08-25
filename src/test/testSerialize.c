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

    p1 = newPacket();
    p1 ->data = calloc(strlen(msg) + 1, sizeof(char));
    memcpy(p1 -> data, msg, strlen(msg) + 1);
    p1->header->dataLen = strlen(msg) + 1;
    p1 ->header ->isAck = true;
    p1 ->header ->endIndex = 20;
    p1 ->header ->index = 3;
    p1 ->header ->msgId = 31;

    buf = serializePacket(p1);
    p2 = deserializePacket(buf);

    sprintf(errorMsg, "testSerialize: expected: %s, actual: %s\n", msg, (char *) p2 ->data);

    assertEquals(
                 msg,
                 p2 ->data,
                 strlen(msg) + 1,
                 errorMsg,
                 cleanup
                 );
    cleanup();
}



