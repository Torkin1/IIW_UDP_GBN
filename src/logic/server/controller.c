#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "logger/logger.h"
#include "dm_protocol/dm_protocol.h"
#include <errno.h>
#include <semaphore.h>
#include <errno.h>
#include <dirent.h>

#define SIZE 1024
#define MAX_CLIENT 5
#define MAX_FILENAME_LENTGH 30

typedef struct Process_info{
        int servers_socket;
        sem_t *s_busy;
        int pid;
 } p_i;

typedef struct node{
   char data[MAX_FILENAME_LENTGH];
   struct node *next;
}list;

void doList(){
    list *head = NULL;
    list *current = NULL;
    head = (list*) malloc(sizeof(list));
    current = (list*) malloc(sizeof(list));
    DIR *d;
    char tooLong[15] = "FileNameTooLong";

    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if((dir->d_type ==DT_REG)){

            logMsg(D, "the name file is :%s\n", dir->d_name);
            if(head == NULL){
                strcpy(head->data, tooLong);
                current = head;
                current ->next = NULL;
            }else{
                while(current != NULL){
                    current = current->next;
                }
                if(strlen(dir->d_name) > MAX_FILENAME_LENTGH){
                    strcpy(current->data, tooLong);
                } else{
                    strcpy(current ->data, dir->d_name);
                }
                current ->next ->next= NULL;
                current = current ->next;
            }

        }
    }
    closedir(d);
  }
}

void operate(p_i *process_index){
    struct sockaddr_in client_addr;
    int n_process = 0;
    socklen_t addr_size;
    int pid;

           logMsg(D, "I'm gonna create the process\n");

    //creating an "array "of pids on the struct p_i(of ALWAYS active process)
    for(n_process; n_process < MAX_CLIENT; n_process++){
        if((pid = fork()) == 0){

            break;
        }
        //The case of the fork failing is not considered.
        //saving the pid on the struct in case i need it
        process_index[n_process].pid = pid;
    }
    int count = 0;
    while(1){
        //define a message that can be used to read the header, so the command
        Message *recived_message;
        count++;
        logMsg(D, "I'm on the while with the pid: %d for the %d\n", process_index[n_process].pid, count);
        int servers_socket = process_index[n_process].servers_socket;
        logMsg(D, "I'm on the while and the server socket is: %d\n", process_index[n_process].servers_socket);

        

        if(pid !=0){
            
            if(receiveMessageDMProtocol(servers_socket, (struct sockaddr *)&client_addr,
            sizeof(client_addr), &recived_message) < 0){
                logMsg(E, "reciveMessageDMProtocol from server: ERROR Couldn`t connect wit hthe client: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            Message *response_message;

            if(recived_message->message_header->command != HS){
                response_message -> message_header ->command = HS;
                response_message -> message_header -> status = OP_STATUS_INCORRECT;
                char response = "Error occured. This port is only for H-S, check it\n";
                response_message ->message_header ->payload_lentgh = strlen(response);
                //NON SICURO DI QUESTO STRLEN, RAGIONARE IN SEGUITO !!!!!!!!!!!! Fare per 8? da ch a bit?
                response_message -> payload = response;
                sendMessageDMProtocol(socket, (struct sockaddr*)&client_addr,
                sizeof(client_addr),response_message);
            }
            else{
                //passare la porta una volta ricevuto l'operazione di H-S del
                //processo disponibile.
                int token_value = 0;
                for(int i = 0; MAX_CLIENT; i++){
                    sem_getvalue(&(process_index[i].s_busy), &token_value);
                    if(token_value == 1){
                        response_message ->message_header -> status = OP_STATUS_OK;
                        in_port_t free_port = DEFAULT_SERVER_PORT + 1 + i;
                        response_message ->message_header ->command = HS;
                        response_message ->message_header ->payload_lentgh = sizeof(in_port_t);
                        response_message ->payload = free_port;
                        break;
                        }
                    else{
                        response_message ->message_header -> status = OP_STATUS_E;
                        response_message ->message_header ->command = HS;
                        char response = "ERROR: No free ports at the moments";
                        response_message -> message_header ->payload_lentgh = strlen(response);
                        response_message ->payload = response;
                    }
                logMsg(E, "HANDSHAKE DONE\n");

                }
                //send the reply  for the HS
                sendMessageDMProtocol(servers_socket, (struct sockaddr*)&client_addr,
                    sizeof(client_addr),response_message);
            }
            destroyMessage(recived_message);
            destroyMessage(recived_message);

        }


        else{
            Message *cmd_msg_request, *cmd_msg_respond;
            cmd_msg_request = newMessage();
            if(receiveMessageDMProtocol(servers_socket, (struct sockaddr *)&client_addr,
            sizeof(client_addr), cmd_msg_request) < 0){
                logMsg(E, "reciveMessageDMProtocol from server: ERROR Couldn`t connect with hthe client: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            logMsg(D, "Recived MSg, I'll lock the semaphore number: %d, with the pid: %d\n", n_process, pid);
            sem_wait(process_index[n_process].s_busy);
            logMsg(D, "I'm after the lock\n");
            //TODO OPERAZIONI ricordare chiusura messaggi socket e sem.
//Domandare a daniele dove si salva il 
            switch (cmd_msg_request->message_header->command)
            {
            case PUT:
                /* code */
                break;
            case GET:
                /* code */
                break;
            case LIST:
                doList();
                break;
            default:
                break;
            } 

        }

    }

}


int start_server(){
    //VARIABLES
    int server_sockfd;
    struct sockaddr_in server_addr;

    p_i process_index[MAX_CLIENT];

            logMsg(D, "Iìll create sockets and things\n");


    //For to open all the socket for the capacity of MAX_CLIENT
    for(int i = 0; i <= MAX_CLIENT; i++){
        if((server_sockfd = socket(AF_INET, SOCK_DGRAM,0)) < 0){
            logMsg(E, "[ERROR][SERVER] Impossible to create the socket %s\n",strerror(errno));
            exit(1);
        }



        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(DEFAULT_SERVER_ADDR);
        server_addr.sin_port = htons(DEFAULT_SERVER_PORT + i);

            logMsg(D, "I'm in a for binding, for the %d time the sockets is: %d\n",i,server_sockfd);


        //Bind the socket
        if((bind(server_sockfd, (struct sockaddr *)&server_addr,
         sizeof(server_addr))) < 0){
            logMsg(E, "[ERROR][SERVER] Bind failed %s\n",strerror(errno));
            exit(1);
         }

         process_index[i].servers_socket = server_sockfd;
         if(i > 0){
             logMsg(D, "Delcaring sempahores %d \n", i, process_index[i].s_busy);
            //dichiaro i semafori
            sem_init(&(process_index[i].s_busy), 1, 1);
            int value;
            sem_getvalue(&process_index[i].s_busy, &value);
            logMsg(D, "I wrote the sempahores, value of %d , is %d\n", i, value);
            process_index[i].servers_socket = server_sockfd;
            logMsg(D, "declared semaphore\n");
         }
    }


    //start the working funciton of the server
    operate(&process_index);
    

    //Desotrying the semaphores
    for(int i = 0; MAX_CLIENT; i++){
        close(&(process_index[i].servers_socket));
        sem_destroy(&(process_index[i].s_busy));
    }
    return 0;
}