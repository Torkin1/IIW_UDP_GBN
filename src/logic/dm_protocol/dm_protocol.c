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


//alla dest_addr_size passo come sizeof (sockaddr_in) cioè ip usato a lvl 3
//serializzo il messaggio e poi lo mando

//problema FIle troppo grandi, se li limito? L'heap non è enorma nache se
//li lascio così, devo tenere una dimensione limitata, oppure un'altra cosa
//metto in head un bool (more) per vedere se ha ricevuto il file lo apre e ci scrive
//i dati, se ci è ancora true allora deve scrivere ulteriormente, finché non è
//falso, aggiungo questo param in message

//nel header di dm_protocol
