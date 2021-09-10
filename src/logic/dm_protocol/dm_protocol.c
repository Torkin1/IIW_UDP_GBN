#include "dm_protocol/message.h"
#include "logger/logger.h"
#include <stdbool.h>
#include <sys/socket.h>

#define DEFAULT_PORT 8888;

int sendMessageDMProtocol(int socket, struct sockaddr *dest_addr,
  socklen_t *dest_addr_size, Message *msg ){

    int message_size = calcMessageSize(msg);

    Message  *message = serializeMessage(msg);
    sendMessageGbn(socket, dest_addr, dest_addr_size, message,
      message_size, NULL);
    logMsg(D, "sendMessageDMProtocol: done\n");
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

    logMsg(D, "reciveMessageDMProtocol: done\n");

  }
