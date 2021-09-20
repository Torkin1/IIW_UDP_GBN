#include "client/controller.h"
#include "dm_protocol/dm_protocol.h"
#include "../testSuites.h"
#include <stdio.h>

static int argc = 2;
static char *argv[] = {"client", "PUT"};
static DmProtocol_command expected = PUT;
static char errorMsg[1024];

void testParseCommandName(){

    DmProtocol_command actual;
    parseCommandName(argc, argv, &actual);

    sprintf(errorMsg, "testParseCommandName: expected: %d, actual: %d\n", expected, actual);

    assertEquals(&expected, &actual, sizeof(DmProtocol_command), errorMsg, NULL);
    
}