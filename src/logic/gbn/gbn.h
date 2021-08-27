#ifndef GBN_H_INCLUDED
#define GBN_H_INCLUDED

#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>

#define LISTENING_PORT

// Errors which can occur during a send
typedef enum sendError {

    MSG_LOST = 1             // Could not assure that the message arrived to the receiver.

} SendError;

// divides a message into packets and adds them to the battery. Calls errorHandler in a new thread if the send went wrong
int sendMessageGbn(int sd, struct sockaddr *dest_addr, socklen_t addrlen, void *msg, int size, void (*errorHandler)(SendError));

#endif // GBN_H_INCLUDED
