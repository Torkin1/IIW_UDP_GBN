#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>




//header of the message, needed for the serialization
typedef struct message_header{
  int commands;
  int status;
  int payload_lentgh;
  bool more;
} MessageHeader;

//The basic info unit used on a message
typedef struct message{
  MessageHeader *message_header; //messages header
  void *payload; //messages payload
} Message;

Message *newMessage();
void destroyMessage(Message *self);

//serialize the message and, returns an array of bytes
uint8_t *serializeMessage(Message *message);

//deserialize the bytes from a array of bytes to a messages
Message *deserializeMessage(uint8_t *bytes);

int calcMsgHeaderSize();
int calcMessageSize(Message *message);

#endif
