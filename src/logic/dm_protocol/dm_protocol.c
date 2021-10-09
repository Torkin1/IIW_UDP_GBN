#include "dm_protocol/dm_protocol.h"
#include "logger/logger.h"
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include "gbn/gbn.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define WAIT_SECS_BEFORE_RETRY 5      // num of secs to wait before retrying to send chunk  

/**
 * Max num of bytes to keep in memory read from a file at a time
 * FIXME: #61. For now, set it to a size ~10kb, better safe than sorry.
*/
#define MAX_FILE_CHUNK_SIZE 10240      

int sendMessageDMProtocol(int socket, struct sockaddr *dest_addr,
  socklen_t dest_addr_size, Message *msg ){

    int message_size = calcMessageSize(msg);

    uint8_t  *buf = serializeMessage(msg);
    sendMessageGbn(socket, dest_addr, dest_addr_size, buf,
      message_size, NULL);
    return 0;
  }


int receiveMessageDMProtocol(int socket, struct sockaddr *sender_addr,
  socklen_t *sender_addr_len, Message **msg){
    void  *message_buff;
    int message_size;
    Message *newMessage;

    recvMessageGbn(socket, sender_addr, sender_addr_len, &message_buff,
       &message_size);
    newMessage = deserializeMessage(message_buff);
    *msg = newMessage;
    return 0;

  }

  int sendFileDMProtocol(int socket, struct sockaddr *dest_addr, socklen_t dest_addr_size, int fileD)
  {

    int bytesRead = 0;
    uint8_t chunk[MAX_FILE_CHUNK_SIZE];
    Message *toSendMsg, *chunkHasArrivedMsg;
    bool more = 0;
    int err;

    lseek(fileD, 0, SEEK_SET);
    do
    {
      while ((bytesRead = read(fileD, chunk, MAX_FILE_CHUNK_SIZE)) < 0)
      {
        err = errno;
        logMsg(E, "sendFileDMProtocol: failed to read chunk from file: %s\n", strerror(err));
      }
      more = (bytesRead == MAX_FILE_CHUNK_SIZE);

      toSendMsg = newMessage();
      toSendMsg->payload = chunk;
      toSendMsg->message_header->more = more;
      toSendMsg->message_header->payload_lentgh = bytesRead;
      toSendMsg ->message_header->command = UNSPECIFIED;

      while (sendMessageDMProtocol(socket, (struct sockaddr *) dest_addr, dest_addr_size, toSendMsg) < 0)
      {
        logMsg(E, "sendFileDMProtocol: failed to send chunk, will retry in %d secs\n", WAIT_SECS_BEFORE_RETRY);
        sleep(WAIT_SECS_BEFORE_RETRY);
      }
      destroyMessage(toSendMsg);

      while (receiveMessageDMProtocol(socket, NULL, NULL, &chunkHasArrivedMsg) < 0 && chunkHasArrivedMsg ->message_header ->command != (int) UNSPECIFIED && chunkHasArrivedMsg->message_header->status != OP_STATUS_OK)
      {
        logMsg(E, "sendFileDMProtocol: an error occurred while listening if chunk has arrived to dest, or a message different from a chunk acknowledgment has been catched and discarded, still listening ...\n");
      }
      logMsg(D, "sendFileDMProtocol: dest received chunk\n");
      destroyMessage(chunkHasArrivedMsg);

    } while (more);

    close(fileD);

    return 0;
  }

  int receiveFileDMProtocol(int socket, struct sockaddr *sender_addr, socklen_t *send_addr_size, int fileD)
  {

    Message *rcvd, *chunkHasArrivedMsg;
    struct sockaddr_in sendAddr = {0};
    socklen_t senderAddrSize;
    bool more;

    // It listens for incoming file chunks. When a chunk arrives, it adds it to the already received chunks, writing the contents in a file.
    do
    {

      while (receiveMessageDMProtocol(socket, (struct sockaddr *)&sendAddr, &senderAddrSize, &rcvd) < 0)
      {
        logMsg(E, "receiveFileDMProtocol: failed to receive message, resuming listening");
      }
      more = rcvd->message_header->more;
      
      lseek(fileD, 0, SEEK_END);
      write(fileD, rcvd->payload, rcvd->message_header->payload_lentgh);
      logMsg(D, "receiveFileDMProtocol: chunk written\n");
      destroyMessage(rcvd);

        chunkHasArrivedMsg = newMessage();
        chunkHasArrivedMsg ->message_header ->status = OP_STATUS_OK;
        while(sendMessageDMProtocol(socket, (struct sockaddr *) &sendAddr, senderAddrSize, chunkHasArrivedMsg) < 0){
          logMsg(E, "receiveFileDMProtocol: an error occurred while trying to inform sender that a chunk has been processed, retrying in %d secs ...\n", WAIT_SECS_BEFORE_RETRY);
          sleep(WAIT_SECS_BEFORE_RETRY);
        }
        destroyMessage(chunkHasArrivedMsg);
     

    } while (more);
    close(fileD);

    if (sender_addr != NULL){
      memcpy(&sendAddr, sender_addr, sizeof(struct sockaddr_in));
    }
    if (send_addr_size != NULL){
      memcpy(send_addr_size, &senderAddrSize, sizeof(socklen_t));
    }
    
    return 0;
  }
