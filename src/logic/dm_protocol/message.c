#include "dm_protocol/message.h"
#include "logger/logger.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>


Message *newMessage(){
  Message *message = calloc(1, sizeof(Message));
  message -> header = calloc(1, sizeof(Header));
}

void destroyMessage(Message *self){
  free(self -> payload);
  free(self -> header);
  free(self);
}


int calcMsgHeaderSize(){
  return 3*sizeof(int) + sizeof(bool);
}

int calcMessageSize(Message *msg){
  return calcMsgHeaderSize() + (msg ->header ->payload_lentgh);
}
uint8_t *serializeMessage(Message *msg){
  void *serialized = calloc(calcMessageSize(msg), sizeof(uint8_t));
  uint8_t *currentByte = serialized;

  //Serializing the message's header
  memcpy(currentByte, &(msg ->header ->commands), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg ->header ->status), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg ->header ->payload_lentgh), sizeof(int));
  currentByte += sizeof(int);
  memcpy(currentByte, &(msg -> header -> more), sizeof(bool));
  currentByte += sizeof(bool);

  //serisalizng the messages payload
  if(msg ->header ->payload_lentgh !=0){
    memcpy(currentByte, msg ->payload, msg ->header ->payload_lentgh);
  }

return serialized;

}

Message *deserializeMessage(uint8_t *bytes){
  Message *deserialized = newMessage();
  uint8_t *currentByte = bytes;

  //Deserializing member of Header structure
  memcpy(&(deserialized ->header ->commands), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->header ->status), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->header ->payload_lentgh), currentByte, sizeof(int));
  currentByte +=sizeof(int);
  memcpy(&(deserialized ->header ->more), currentByte, sizeof(bool));
  currentByte += sizeof(bool);

//Deserialized data are store in a buffer allocated dynamically
  if(deserialized -> header -> payload_lentgh != 0){
    deserialized -> payload = calloc(deserialized -> header ->payload_lentgh, sizeof(uint8_t));
    memcpy(deserialized ->payload, currentByte, deserialized -> header ->payload_lentgh);
  }
}
