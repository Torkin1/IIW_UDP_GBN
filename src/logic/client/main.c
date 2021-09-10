#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../logger/logger.h"


#define SIZE 1024 //un generico buffersieze

 int main(){

  // Scrivo ip e porta, stessa tra server e client
  char *ip = "127.0.0.1";
  int port = 8080;

// Definisco le variabili
  int server_sockfd; //descrittore del socket
  struct sockaddr_in server_addr;//struttura socket
  FILE *fp;//descrittore del file
  char *filename = "panda";//nome file che creo
  // Creao la socket UDP, si usano le sock_dgram sempre IPv4, con IP (lo 0 finale)
    server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sockfd < 0){
      perror("[ERRORE] impossibile creare la socket");
      exit(1);
    }
  //struttura della socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  // Leggento il ifle client.txt, demo da cancellare TODO
  fp = fopen(filename, "r");
  if (fp == NULL){
      perror("[ERRORE] Impossibile leggere il file");
      exit(1);
  }

    // Invio del file al server
  send_file_data(fp, server_sockfd, server_addr);

  //Dei print per chiarezza
  printf("[SUCCESSO] transferimento dei dati completato.\n");
  //TODO metterlo in un while e aspettare sempre comandi, tra cui chiusura
  printf("[Chiusura] Disconnessioen dal server.\n");

  close(server_sockfd);
  return 0;
}

//Invio di un file, con fp cioè descrittore del file, socketfd è la scoket e la
//sua struttura, del client
void send_file_data(FILE *fp, int sockfd, struct sockaddr_in addr){
  //variabili locali
  int n;
  char buffer[SIZE];

  // Invio dati
  while(fgets(buffer, SIZE, fp) != NULL){//leggo il file con un while
      printf("[INVIO] Data: %s", buffer);//stampo cosa leggo
      logMsg(I, "[INVIO] Data: %s\n", index);


      //Invio i dati con sendto
      n = sendto(sockfd, buffer, SIZE, 0, (struct sockaddr*)&addr, sizeof(addr));
      if (n == -1){
        perror("[ERRORE] impossibile inviare dato al server.");
        exit(1);
      }
      bzero(buffer, SIZE);//azzero il buffer

    }

    // Invio dell'"END" per poter definire la fine del file, fare così o meglio
    //come se fosse un comando la prossima volta, preso da input del client
    strcpy(buffer, "END");
    sendto(sockfd, buffer, SIZE, 0, (struct sockaddr*)&addr, sizeof(addr));
    fclose(fp);
    return;
}
