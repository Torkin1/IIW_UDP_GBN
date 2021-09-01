#ifndef CATCHER_H_INCLUDED
#define CATCHER_H_INCLUDED

#include <sys/socket.h>

// implements listening loop and message deliver described by recvMessageGbn()
int listenForData(int sd, struct sockaddr *senderAddr, socklen_t *senderAddrlen, void **msgBuf, int *size);

#endif // CATCHER_H_INCLUDED