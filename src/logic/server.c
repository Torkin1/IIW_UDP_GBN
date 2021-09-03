#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //per il multi-threading, aggiungere  "-lpthread" per farlo funzionare

#define SERVER_MSG_DIM 2000
#define SERVER_PORT_NUMBER 8888

//La funzione del handler (cioè thread)
void *connection_handler(void *);

//Inizio main
int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;//variabili per la creazione delle socket
	struct sockaddr_in server , client;//Struttura di una socket (possibile castarla a sockaddr senza
	//problemi). Qui fatta sia per client che server.

	//Crea la socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)//controllo se la creazione non è andata a buon fine
		printf("ERRORE: IMPOSSIBILE creare la socket");
	//Se va a buon fine uso puts così va a capo direttamente.
	puts("Socket createa con successo");

	//Compilo i campi della struct di sockaddr_in del server
	server.sin_family = AF_INET; //usiamo indirizzi ipv4
	server.sin_addr.s_addr = INADDR_ANY;//con INADDR_ANY farò si che la socket si leghi
	//a una porta libera a caso, con l'indirizzo definito in INADDR_ANY
	server.sin_port = htons(SERVER_PORT_NUMBER);//assegno la porta 8888 e lo faccio sfruttando htons per
	//evitare possibili errori di lettura, in base all'ordine, così ognuno lo leggerà a moodo suo.

	//Bind della socket con dimensione di server
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//In caso di errore
		perror("Il bind ha fallito, riprova");
		return 1;
	}
	puts("bind completato");

	//Listen, ascolta se ci sono richieste in corso
	listen(socket_desc , 3);

	//Accetta connesioni possiibli
	puts("Sono il server, ed attendo connesioni...");
	c = sizeof(struct sockaddr_in);
	//loop per ascoltare sempre se ci sono nuovi clinet a connettersi
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{//se un client si è connesso, verrà riferita la connessione
		puts("connessione accettata");

		pthread_t listener_thread;//creo un listener
		new_sock = malloc(1); //nuova socket alloc 1 byte di memoria
		*new_sock = client_sock; //la nuova socket la assegno come quella del client
		if( pthread_create( &listener_thread , NULL ,  connection_handler , (void*) new_sock) < 0)//credo un
		{//nuovo thread e come routine dovrà svolgere connection_handler() con argomento new_sock
			perror("ERRORE: IMPOSSIBILE creare il thread");
			return 1;
		}

		puts("Assegnato all'handler");
	}

	if (client_sock < 0)//controllo se la creazione del socket del client è stato fatto correttamente
	{
		perror("l'accept ha fallito");
		return 1;
	}
	pthread_exit(NULL);//uccido il thread
	return 0;
}

/*
 *Questo sarà l'handler delle connessioni dei vari client
*/
void *connection_handler(void *socket_desc)
{
	//Salvo il descrittore del socket
	int sock = *(int*)socket_desc;
	//Da cambiare quando si inviano file!
	int read_size;//variabile che verrà usata per salvare la dimensione del messaggio inviato dal client
	char *message , client_message[SERVER_MSG_DIM]; //salvo come char il puntatore del messaggio e un vettore di
	//caratteri da 2000.


	//Lettura messaggi dal client
	while( (read_size = recv(sock , client_message , SERVER_MSG_DIM , 0)) > 0 )//limito il messaggio a 2000 bit
	{
		//Invio del messaggio ricevuto al client
		//Una volta ricevuto il messaggio lo rispedisco al client
		message = client_message;
		write(sock , message , strlen(message));
		message = "\nSalve sono il tuo Handler, mi occuperò della tua connesione con il server\n";
		write(sock , message , strlen(message));

		message = "Scrivi qualcosa e io la ripeterò, non è incredibile? \n";
		write(sock , message , strlen(message));

		memset(client_message,'\0',sizeof(client_message));

	}

	if(read_size == 0)
	{
		puts("Client disconnesso");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv ha fallito");
	}

	//uso la funzioen free sul puntato del descrittore della socket
	free(socket_desc);

	return 0;
}
