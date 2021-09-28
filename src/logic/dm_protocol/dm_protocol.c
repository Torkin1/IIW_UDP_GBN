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
#define MAX_FILE_CHUNK_SIZE 10240     // max num of bytes to keep in memory read from a file at a time

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

  int sendFileDMProtocol(int socket, struct sockaddr *dest_addr, socklen_t dest_addr_size, char* pathName)
  {

    int fileD;
    int bytesRead = 0;
    uint8_t chunk[MAX_FILE_CHUNK_SIZE];
    Message *toSendMsg, *chunkHasArrivedMsg;
    bool more;
    
    fileD = open(pathName, O_RDONLY | O_NONBLOCK);
    lseek(fileD, 0, SEEK_SET);
    do
    {
      if ((bytesRead = read(fileD, chunk, MAX_FILE_CHUNK_SIZE)) < 0)
      {
        int err = errno;
        logMsg(E, "sendFileDMProtocol: failed to read chunk from file: %s\n", strerror(err));
        return -1;
      }
      // FIXME: l'invio di diversi chunk comporta diversi bug, tra cui seg faults, arrivo disordinato dei chunks, ...
      //more = (bytesRead == MAX_FILE_CHUNK_SIZE);

      toSendMsg = newMessage();
      toSendMsg->payload = chunk;
      toSendMsg->message_header->more = more;
      toSendMsg->message_header->payload_lentgh = bytesRead;

      logMsg(D, "more is %d\n", more);
      while (sendMessageDMProtocol(socket, dest_addr, dest_addr_size, toSendMsg) < 0)
      {
        logMsg(E, "sendFileDMProtocol: failed to send chunk, will retry in %d secs\n", WAIT_SECS_BEFORE_RETRY);
        sleep(WAIT_SECS_BEFORE_RETRY);
      }
      destroyMessage(toSendMsg);

      while (receiveMessageDMProtocol(socket, NULL, NULL, &chunkHasArrivedMsg) < 0 && chunkHasArrivedMsg->message_header->status != OP_STATUS_OK)
      {
        logMsg(E, "sendFileDMProtocol: an error occurred while listening if chunk has arrived to dest, still listening ...\n");
      }
      logMsg(D, "sendFileDMProtocol: dest received chunk\n");
      destroyMessage(chunkHasArrivedMsg);

    } while (more);

    close(fileD);

    return 0;
  }

  int receiveFileDMProtocol(int socket, struct sockaddr *sender_addr, socklen_t *send_addr_size, char *fileName)
  {

    Message *rcvd, *chunkHasArrivedMsg;
    struct sockaddr_in sendAddr = {0};
    socklen_t senderAddrSize;
    int fileD;
    bool more;

    // file is overwritten if already exists
    if ((fileD = open(fileName, O_APPEND | O_WRONLY | O_CREAT | O_TRUNC)) < 0)
    {
      int err = errno;
      logMsg(E, "receiveFileDMProtocol: failed to create file: %s\n", strerror(err));
      return -1;
    }

    logMsg(D, "receiveFileDMProtocol: file opened\n");

    // It listens for incoming file chunks. When a chunk arrives, it adds it to the already received chunks, writing the contents in a file.
    do
    {

      while (receiveMessageDMProtocol(socket, &sendAddr, &senderAddrSize, &rcvd) < 0)
      {
        logMsg(E, "receiveFileDMProtocol: failed to receive message, resuming listening");
      }
      more = rcvd->message_header->more;
      
      write(fileD, rcvd->payload, rcvd->message_header->payload_lentgh);
      logMsg(D, "receiveFileDMProtocol: chunk written\n");
      destroyMessage(rcvd);

    
        chunkHasArrivedMsg = newMessage();
        chunkHasArrivedMsg ->message_header ->status = OP_STATUS_OK;
        while(sendMessageDMProtocol(socket, &sendAddr, senderAddrSize, chunkHasArrivedMsg) < 0){
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
