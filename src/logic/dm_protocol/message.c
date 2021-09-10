#include "dm_protocol/message.h"
#include "logger/logger.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>


Message *newMessage(){
  Message *message = calloc(1, sizeof(Message));
  message -> message_header = calloc(1, sizeof(MessageHeader));
}

void destroyMessage(Message *self){
  free(self -> payload);
  free(self -> message_header);
  free(self);
}


int calcMsgHeaderSize(){
  return 3*sizeof(int) + sizeof(bool);
}

int calcMessageSize(Message *msg){
  return calcMsgHeaderSize() + (msg ->message_header ->payload_lentgh);

}
uint8_t *serializeMessage(Message *msg){
  void *serialized = calloc(calcMessageSize(msg), sizeof(uint8_t));
  uint8_t *currentByte = serialized;

  //Serializing the message's message_header
  memcpy(currentByte, &(msg ->message_header ->commands), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg ->message_header ->status), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg ->message_header ->payload_lentgh), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg -> message_header -> more), sizeof(bool));
  currentByte += sizeof(bool);

  //serisalizng the messages payload
  if(msg ->message_header ->payload_lentgh !=0){
    memcpy(currentByte, msg ->payload, msg ->message_header ->payload_lentgh);
  }

  return serialized;

}

Message *deserializeMessage(uint8_t *bytes){
  Message *deserialized = newMessage();
  uint8_t *currentByte = bytes;

  //Deserializing member of MessageHeader structure
  memcpy(&(deserialized ->message_header ->commands), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->message_header ->status), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->message_header ->payload_lentgh), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->message_header ->more), currentByte, sizeof(bool));
  currentByte += sizeof(bool);

//Deserialized data are store in a buffer allocated dynamically
  if(deserialized -> message_header -> payload_lentgh != 0){
    deserialized -> payload = calloc(deserialized -> message_header ->payload_lentgh, sizeof(uint8_t));
    memcpy(deserialized ->payload, currentByte, deserialized -> message_header ->payload_lentgh);
  }
  return deserialized;
}
