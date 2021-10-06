#ifndef SERVER_CONTROLLER
#define SERVER_CONTROLLER

#include "dm_protocol/dm_protocol.h"

int parseCommandName(int argc, char *argv[], DmProtocol_command *toInvoke);

int start_server();
#endif