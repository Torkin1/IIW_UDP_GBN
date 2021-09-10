#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "logger/logger.h"

#define SIZE 1024

//Funzione chiamata per la scrittura del file, da sosstituire con una per la lettura di comandi
void write_file(int sockfd, struct sockaddr_in addr){//la struct è quella del client
  FILE *fp;
  char *filename = "panda_server";
  int n;
  char buffer[SIZE];
  socklen_t addr_size;

  // Creazione file
  fp = fopen(filename, "w");

  // Ricevendo i dati e scrivedoli su file
  while(1){

    addr_size = sizeof(addr);
    n = recvMessageGbn(sockfd, buffer, SIZE, 0, (struct sockaddr*)&addr, &addr_size);

/*
int size;

Seriliazzo come?
ho io lenght del msg
status = --

uint_8 buf = serializeMSG(msg);
buf lo passo a SendMessageGbn, che si prende m, che m è la sua lunghezza

Deserializzo, la prendo da SendMessageGbn size, cioè ho una int rcvd sieze e poi
&rcvdMSG e poi la uso
*/
    if (strcmp(buffer, "END") == 0){
      break;
      return;
    }

    printf("[RICEVUTO] Data: %s", buffer);
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);

  }

  fclose(fp);
  return;
}

/*
GET:
Client fa l'invito del file con payload grandezza e status non server,
il server salva il file richiesto dal client, status ok se è andato a buon fine o altri errori  e lo invia al client, e payload_lentgh
LIST: Non mi serve il payload non mi serve il payload è sempre di tutti i file, lenght è 0,  roba... ricorda (vedere serialize e deserialize #include "packet).")
*/


/*


typedef enum commands {
  list,
  put,
  get,
  numero_comandi
}command
*/

/*

comando 4 byte (int)
status 4 byte (int)
payload ? VARIABILE attributo nuovo, cioè payload_lentgh
payload_lentgh 4 byte.
payload
DA STD INT USO uint8_t buf[]

PUT: Client richede il salvataggio del file in payload sul server, Server salva il file da payload
su disco e invia status

*/
int main(){

  // Scrivo porta e indirizzo ip, stesso del CLIENT!!!
  char *ip = "127.0.0.1";
  int port = 8080;

  // Variabili
  int server_sockfd;
  struct sockaddr_in server_addr, client_addr;
  char buffer[SIZE];
  int e;

  // Creazione della socket, per IPv4, che sia per UDP, su protocollo IP.
  server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_sockfd < 0){
    perror("[ERRORE] Impossibile crare la socket");
    exit(1);
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

//Bind tra socket e IP e porta
  e = bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (e < 0){
		// logMsg(E, "[ERROR] servers bind: bind faild check the port %d, and ip %d\n", port, ip);
    exit(1);
  }

	logMsg(I, "[INIZIO] Start del server UDP\n");
  printf("[INIZIO] Start del server UDP\n");
  //TODO qui uso write file per scrivere sul file e ricevere il file
  write_file(server_sockfd, client_addr);

  printf("[SUCCESSO] Trasferimento completato.\n");
  printf("[CHIUSURA] Chiusura server in corso...\n");

  close(server_sockfd);

  return 0;
}
//strerror
// logMsg(D, "packetize: max data len is %d\n", calcMaxDataLen());
