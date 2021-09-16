#include "dm_protocol/message.h"
#include "logger/logger.h"
#include <stdbool.h>
#include <sys/socket.h>
#include "gbn/gbn.h"

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

