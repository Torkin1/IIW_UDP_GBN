#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "dm_protocol/dm_protocol.h"

/**
    Creates a dynamically allocated hashtable and populates it with op codes using their names as keys, if not created before.
    @return Command code corresponding to given name. Returns NULL if no command is registered with such name
*/
DmProtocol_command *getCommandByName(char *name);

/**
    Creates socket usable by all client components if not created before, then it returns it.
    Socket shall be created conforming to the protocol used (i.e. using dmProtocol implies creating a UDP internet socket)
    Socket can be closed using close() as usual.
    @return socket descriptor if successful, else -1
*/
int getGlobalSocket();

/**
	dynamically allocates a struct holding server default address, if it has not been created before.
	@return default server address.
*/
struct sockaddr *getServerAddress();

/**
    Extracts command name from command line and retrieves the op code corresponding to extracted name if there is one, else returns an error
    @return 0 on success, else -1
*/
int parseCommandName(int argc, char *argv[], DmProtocol_command *toInvoke);

/*
    calls routine corresponding to given command code, if there is one.
    Only implemented operations shall have a routine to be called.
    @return 0 on success, else -1
*/
int doCommand(DmProtocol_command toInvoke, char *toInvokeArgs[]);

#endif // CONTROLLER_H_INCLUDED