#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

#define SERVER_MSG_DIM 2000
#define CLIENT_MSG_DIM 1000
#define SERVER_PORT_NUMBER 8888

/*Inizio main*/
int main(int argc , char *argv[])
{
	int sock;//var di socket
	struct sockaddr_in server; //la struct della socket del server
	char message[CLIENT_MSG_DIM] , server_reply[SERVER_MSG_DIM];//var con messaggio di client e server, da togliere quando combinate
	memset(server_reply, 0, SERVER_MSG_DIM);

	//Creo la socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) //controollo creazione corretta socket
	{
		printf("ERRORE: IMPOSSIBILE creare la socket");
	}
	puts("Socket creata con successo");//puts invece di printf per poter andare a campo in automatico

	//Completo la struct della socket del server
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT_NUMBER);

	//Tenta di connettersi al server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("ERRORE: Connessione fallita");//In caso di errore di connessione (Es. server giu')
		return 1;
	}

	puts("Connessa\n");

	//Mantengo la connessione aperta con il server
	while(1)
	{
		printf("Scrivi il messaggio: ");
		scanf("%s" , message);

		//Invia i dati (in questo caso il messaggio)
		if( send(sock , message , strlen(message) , 0) < 0)
		{
			puts("ERRORE: Invio messaggio fallito");
			return 1;
		}

		//Uso recv per leggere i messaggi ricevuti da parte del server
		if( recv(sock , server_reply , SERVER_MSG_DIM , 0) < 0)
		{
			puts("ERRORE: recv ha fallito");
			break;
		}

		puts("Risposta del server:");
		puts(server_reply);

	}

	close(sock);//chiudo la socket
	return 0;
}
