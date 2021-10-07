#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "logger/logger.h"
#include "dm_protocol/dm_protocol.h"
#include "dataStructures/ll.h"
#include <errno.h>
#include <semaphore.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SIZE 1024
#define MAX_CLIENT 5
#define MAX_FILENAME_LENTGH 256
char *program_name;


typedef struct Process_info{
        int servers_socket;
        sem_t s_busy;
        int pid;
 } p_i;

typedef struct node{
   char data[MAX_FILENAME_LENTGH];
   struct node *next;
}list;

int doPut(struct sockaddr *client_addr, char *filename, int sockfd){
    Message *response;
    int fd;

    if(filename == NULL){//Controll tyhat probably isn't necessary
        logMsg(E, "[Server] doPut: fileName can't be NULL\n");
        return -1;
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    response = newMessage();


    //Controllare se non metto la payload lentgh se ho come OP_STAT_OK
    response ->message_header ->status = OP_STATUS_OK;
    response ->message_header ->command = PUT;
    response ->message_header ->payload_lentgh = strlen(filename);
    response ->payload = filename;
    if(sendMessageDMProtocol(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr),response) != 0){
            logMsg(E, "[SERVER]sendMessageDMProtocol on doGet: ERROR Couldn`t send to the client: %s\n", strerror(errno));
            destroyMessage(response);
            return -1;
    }

    if(receiveFileDMProtocol(sockfd, NULL, NULL, fd)){
        response ->message_header ->status = OP_STATUS_OK;
        response ->message_header ->command = PUT;
        response ->message_header ->payload_lentgh = strlen(filename);
        response ->payload = filename;
        /* //implement if needed
        if(sendMessageDMProtocol(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr),response) != 0){
            logMsg(E, "[SERVER]recieveFileDMProtocol on doPut: ERROR nothing recieved from the client: %s\n", strerror(errno));
            destroyMessage(response);
            return -1;
        }*/
        logMsg(E, "[SERVER]: doPut failed to receive file content from the client\n");
        return -1;
    }

    
    return 0;
}

int doGet(struct sockaddr *client_addr, char *filename, int sockfd){
    int fd, err;
    Message *respond;

    //filename must be not NULL
    if(filename == NULL){
        logMsg(E, "[SERVER], doGet have a filename == NULL\n");
        return -1;
    }

    //Sending the file to the client
    respond = newMessage();
    //opens the file
    if((fd = open(filename, O_RDONLY) )){
        err = errno;
        logMsg(E, "[SERVER], doGet: failed to open the file %s: %s\n", filename, strerror(err));
        respond ->message_header ->command = GET;
        char *errorMessage = "[SERVER] Couldn't open the file\n";
        respond ->message_header ->payload_lentgh = strlen(errorMessage);
        respond  ->payload = errorMessage;
        respond ->message_header ->status = OP_STATUS_E_FILE_NOT_FOUND;
        if(sendMessageDMProtocol(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr),respond) != 0){
            logMsg(E, "[SERVER]sendMessageDMProtocol on doGet: ERROR Couldn`t send to the client: %s\n", strerror(errno));
            destroyMessage(respond);
            return -1;
        }
        destroyMessage(respond);
        return -1;
    }

    

    //respond to the command
    respond ->message_header ->command = GET;
    respond ->message_header ->payload_lentgh = strlen(filename) + 1;
    respond ->message_header ->status = OP_STATUS_OK;
    if(sendMessageDMProtocol(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr),respond) != 0){
        logMsg(E, "[SERVER]sendMessageDMProtocol on doGet: ERROR Couldn`t send to the client: %s\n", strerror(errno));
        destroyMessage(respond);
        return -1;
    }

    if(sendFileDMProtocol(sockfd, (struct sockaddr *) &client_addr,sizeof(client_addr), respond ) !=0){
        
        logMsg(E, "[SERVER] sendFile in doGet failed\n");
        return -1;
    }

    return 0;
}

int doList(struct sockaddr *client_addr,char *separator, int sockfd){
    Node *list = NULL;
    DIR *d;
    char tooLong[15] = "FileNameTooLong";
    int strings_len = 0;
    int files_number = 0;

    logMsg(D,"[SERVER-DOLIST] I'm starting lol\n");
    struct dirent *dir;
    d = opendir(".");//open directory's path
    if (d) {
        while ((dir = readdir(d)) != NULL) {//return 0 if there's a file in the dir
            if((dir->d_type ==DT_REG)){//check if the file is not a directory
                
                files_number++;//increase the file number, used to allocate memory later
                strings_len = strings_len + strlen(dir->d_name);//var with the sum of any filename
                logMsg(D, "the name file is :%s\n", dir->d_name);

                if(strlen(dir->d_name) > MAX_FILENAME_LENTGH){
                        strings_len = strings_len + strlen(tooLong) - strlen(dir->d_name);
                        appendLL(&list, tooLong);
                    } else{
                        appendLL(&list, dir->d_name);
                    }
                }

        }
    }
    closedir(d);
    //sending the list
    if(strings_len == 0){

    }
    char *payload_string;
    int dim = 0;
    
    if(files_number > 0){
        dim = files_number*(strlen(separator)) + strings_len + 1;
        payload_string = (char*)malloc(dim);
        memset(payload_string, '\0', dim);
        
    }
    else{
        payload_string = separator;
        dim = strlen(separator);
    }

    Node *element = newNode(NULL);
    for(int i = 0; i < files_number; i++){
        getLL(list, i, &element);
        logMsg(D, "[doList]: Hi the node value is %s and the argv is: %s\n", element->value, program_name);
        if((strcmp(element->value, program_name)) == 0){
            continue;
        }
        strcat(payload_string, element->value);
        strcat(payload_string, separator);
        logMsg(D, "[doList]: Hi the payload_string is %s\n", payload_string);

    }

    //sending the message
    Message *list_msg = newMessage();
    list_msg->payload = payload_string;
    list_msg->message_header->payload_lentgh = strlen(payload_string);
    list_msg->message_header->command = LIST;
    list_msg->message_header->status = OP_STATUS_OK;
    if(sendMessageDMProtocol(sockfd, (struct sockaddr*)client_addr, sizeof(*client_addr),list_msg) != 0){
        logMsg(E, "sendMessageDMProtocol from server on doList: ERROR Couldn`t send to the client: %s\n", strerror(errno));
        return -1;
   }
   return 0;

}

void operate(p_i *process_index){
    struct sockaddr_in client_addr;
    int n_process = 1;
    socklen_t addr_size;
    int pid;

           logMsg(D, "I'm gonna create the process\n");

    //creating an "array "of pids on the struct p_i(of ALWAYS active process)
    for(n_process; n_process <= MAX_CLIENT; n_process++){
        if((pid = fork()) == 0){
            //The case of the fork failing is not considered.
        //saving the pid on the struct in case i need it
            process_index[n_process].pid = pid;
            
            break;
        }  
    }
    int count = 0;
    while(1){
        //define a message that can be used to read the header, so the command
        Message *recived_message;
        count++;
        int servers_socket = process_index[n_process].servers_socket;

        

        if(pid !=0){
            logMsg(D, "[SERVER] Father process waiting for the HS and my scoket is:%d for the %d\n", process_index[0].servers_socket, count);
            if(receiveMessageDMProtocol(process_index[0].servers_socket, (struct sockaddr *)&client_addr,
            &addr_size, &recived_message) < 0){
                logMsg(E, "reciveMessageDMProtocol from server: ERROR Couldn`t connect wit hthe client: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            Message *response_message;
            response_message = newMessage();

            if(recived_message->message_header->command != HS){
                response_message -> message_header ->command = HS;
                response_message -> message_header -> status = OP_STATUS_INCORRECT;
                char *response = "Error occured. This port is only for H-S, check it\n";
                response_message ->message_header ->payload_lentgh = strlen(response);
                //NON SICURO DI QUESTO STRLEN, RAGIONARE IN SEGUITO !!!!!!!!!!!! Fare per 8? da ch a bit?
                response_message -> payload = response;
                sendMessageDMProtocol(servers_socket, (struct sockaddr*)&client_addr,
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
                        response_message ->payload = &free_port;
                        break;
                        }
                    else{
                        response_message ->message_header -> status = OP_STATUS_E;
                        response_message ->message_header ->command = HS;
                        char *response = "ERROR: No free ports at the moments";
                        response_message -> message_header ->payload_lentgh = strlen(response)+1;
                        response_message ->payload = response;
                    }

                }
                //send the reply  for the HS
                
                if(sendMessageDMProtocol(process_index[0].servers_socket, (struct sockaddr*)&client_addr, sizeof(client_addr),response_message) != 0){
                    logMsg(E, "sendMessageDMProtocol from server on doList: ERROR Couldn`t send to the client: %s\n", strerror(errno));
                }

            }
            destroyMessage(response_message);

        }


        else{
            logMsg(D, "I'm on the while with the pid: %d for the %d time, my n_process is: %d my socket is:%d\n", process_index[n_process].pid, count, n_process, process_index[n_process].servers_socket);
            Message *cmd_msg_respond;
            
            logMsg(D, "[Server] Hi, I'm the child, my socket is:%d \n", servers_socket);

            if(receiveMessageDMProtocol(servers_socket, (struct sockaddr *)&client_addr,
            &addr_size, &cmd_msg_respond) < 0){
                logMsg(E, "reciveMessageDMProtocol from server: ERROR Couldn`t connect with hthe client: %s\n", strerror(errno));
            }

            logMsg(D, "[SERVER-CHILD]Recieved MSg, I'll lock the semaphore number: %d, with the pid: %d for the %d\n", n_process, process_index[n_process].pid, count);
            if((sem_wait(&(process_index[n_process].s_busy)))<0){
                logMsg(E, "The sema_wait failed! %s",strerror(errno));
            }
            logMsg(D, "I'm after the lock\n");
            //TODO OPERAZIONI ricordare chiusura messaggi socket e sem.
//Domandare a daniele dove si salva il 
            switch (cmd_msg_respond->message_header->command)
            {
            /*case PUT:
                if((doPut((struct sockaddr *)&client_addr, cmd_msg_respond->payload, servers_socket)) != 0){
                    logMsg(E, "doPut from server failed");
                }
                break;
            case GET:
                if((doGet((struct sockaddr *)&client_addr, cmd_msg_respond->payload, servers_socket)) != 0){
                    logMsg(E, "doGet from server failed");
                }
                break;*/
            case LIST:
                if(doList((struct sockaddr *)&client_addr, cmd_msg_respond->payload,servers_socket) != 0){
                    logMsg(E, "doList from server failed");

                }
                break;
            default:
                break;
            } 
            //Semaphore of the n_process to 1 again
            sem_post(&(process_index[n_process].s_busy));
            logMsg(D, "[SERVER] Semaphores to 1\n");


        }

    }

}


int main(int argc, char *argv[]){
    //VARIABLES
    int server_sockfd;
    struct sockaddr_in server_addr;

    program_name = argv[0] + 2;
    p_i *process_index;
    
    if((process_index = mmap(NULL, MAX_CLIENT*sizeof(p_i), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == (void*)-1){
        logMsg(E,"SERVER-MAIN, failed the mmap %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    logMsg(D, "I'll create sockets and things\n");


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
             logMsg(D, "Delcaring sempahores %d \n", i, &(process_index[i].s_busy));
            //dichiaro i semafori
            sem_init(&(process_index[i].s_busy), 1, 1);
            int value;
            sem_getvalue(&(process_index[i].s_busy), &value);
            logMsg(D, "I wrote the sempahores, value of %d , is %d\n", i, value);
            process_index[i].servers_socket = server_sockfd;
            logMsg(D, "declared semaphore\n");
         }
    }


    //start the working funciton of the server
    operate(process_index);
    

    //Desotrying the semaphores
    for(int i = 0; MAX_CLIENT; i++){
        close((process_index[i].servers_socket));
        sem_destroy((&(process_index[i].s_busy)));
    }
    return 0;
}