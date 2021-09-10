#include "dm_protocol/message.h"
#include "logger/logger.h"
#include <stdbool.h>

int sendMessageDMProtocol(int socket, struct sockaddr *dest_addr,
  socklen_t *dest_addr_size, Message *msg ){
    int message_size = calcMessageSize(msg);
    serializeMessage(msg);
    sendMessageGbn(socket, dest_addr, dest_addr_size, msg,
      message_size, NULL);
    logMsg(D, "sendMessageDMProtocol: done\n");
  }


int reciveMessageDMProtocol(int socket, struct sockaddr *sender_addr,
  socklen_t *sender_addr_len, Message *msg){
    int message_size;
    recvMessageGbn(socket, sender_addr, sender_addr_len, *msg,
       message_size);
    deserializeMessage(msg);

    logMsg(D, "reciveMessageDMProtocol: done\n");

  }
