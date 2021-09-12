#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//header of the message, needed for the serialization
typedef struct message_header{
  
  /*
      Code meaning what operation the message is about.
      Client sets command with the code of the operation that the server must execute.
      Server sets command with the code of what operation the result is about.
  */
  int command;

  /*
      Code set by server to tell if the client request has been fulfilled or not.
      Status codes and their meaning shall be defined by operations.
  */
  int status;

  int payload_lentgh;     // how many bytes is the payload
  bool more;              // true if further chunks of data must be expected in payloads of following messages 
  
} MessageHeader;

//The basic info unit used on a message
typedef struct message{
  MessageHeader *message_header;      // message metadata
  
  /*
      Message data.
      Client sets payload as data needed by the server to perform requested operation.
      Server sets payload with data describing errors if it couldn't perform the operation.
      Such data and its format shall be defined by the status.
  */
  void *payload;
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
