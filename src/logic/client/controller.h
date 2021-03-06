#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "dm_protocol/dm_protocol.h"

/**
    Extracts command name from command line and retrieves the op code corresponding to extracted name if there is one, else returns an error
    @return 0 on success, -1 if number of args is incorrect, or no command code is registered with given name
*/
int parseCommandName(int argc, char *argv[], DmProtocol_command *toInvoke);

/*
    calls routine corresponding to given command code, if there is one.
    Only implemented operations shall have a routine to be called.
    @return 0 on success, -1 if no routine is registered to given command, or the command invoked failed
*/
int doCommand(DmProtocol_command toInvoke, char *toInvokeArgs[]);

#endif // CONTROLLER_H_INCLUDED