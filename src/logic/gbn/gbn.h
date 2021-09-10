#ifndef GBN_H_INCLUDED
#define GBN_H_INCLUDED

#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>

// Errors which can occur during a send
typedef enum sendError {

    MSG_LOST = 1             // Could not assure that the message arrived to the receiver.

} SendError;

/*
    divides a message into packets and atomically adds them to the battery. Packets will be launched asynchronously by a launcher thread.
    @param sd socket used to send packets
    @param dest_addr destination address
    @param addrlen dest addr size
    @param msg pointer to message to send
    @param size size of message
    @param errorHandler function called on a new thread if the send went wrong
    @return 0 if success, else -1
*/
int sendMessageGbn(int sd, struct sockaddr *dest_addr, socklen_t addrlen, void *msg, int size, void (*errorHandler)(SendError));

/*
    Listens for packets. When a packet is arrived, waits for all other packets of the same message to arrive, extracts data from each packet and builds the original message. Returns when the entire message is built.
    @param sd socket used to listen packets. Must be already bound.
    @param senderAddr if not NULL, sender addr will be copied on *senderAddr
    @param senderAddrlen if not NULL, sender addr len is stored on *senderAddrLen
    @param msgBuf pointer to dynamically allocated message received will be stored in *msgBuf. Must be not NULL
    @param size size of message received will be stored in *size. Must be not NULL
    @return 0 if success, else -1
*/
int recvMessageGbn(int sd, struct sockaddr *senderAddr, socklen_t *senderAddrlen, void **msgBuf, int *size);

#endif // GBN_H_INCLUDED
