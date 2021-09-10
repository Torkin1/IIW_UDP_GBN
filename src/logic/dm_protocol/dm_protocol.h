#ifndef DM_PROTCOL_H_INCLUDED
#define DM_PROTCOL_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include "dm_protocol/message.h"
#include <sys/socket.h>

typedef enum commandsList{
  PUT,
  GET,
  LIST,
  HS,
  COMMANDS_NUM
}CommandsList;


/*
  divides a message into an array and sendend to the sendMessageGbn
  @param sd socket used to send packets
  @param dest_addr destination address
  @param dest_addr_size dest addr size
  @param msg pointer to message to send
  @return 0 if success, else -1
*/
int sendMessageDMProtocol(int socket, struct sockaddr *dest_addr,
  socklen_t *dest_addr_size, Message *msg );

/*
  returns if we recived a message this is called
  @param sd socket used to listen packets. Must be already bound
  @param senderAddr if not NULL, sender addr will be copied on *senderAddr
  @param senderAddrlen if not NULL, sender addr len is stored on *sender_addr_len
  @param msg pointer to message to send
  @return 0 if success, else -1
*/


int receiveMessageDMProtocol(int socket, struct sockaddr *sender_addr,
  socklen_t *sender_addr_len, Message **msg);

#endif
