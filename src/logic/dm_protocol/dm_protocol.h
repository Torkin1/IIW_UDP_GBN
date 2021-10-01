#ifndef DM_PROTCOL_H_INCLUDED
#define DM_PROTCOL_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include "dm_protocol/message.h"
#include <sys/socket.h>
#include <limits.h>

// default port where the server will listen for handshakes
#define DEFAULT_SERVER_PORT 8888
#define DEFAULT_SERVER_ADDR INADDR_LOOPBACK

// all operations that server and client must support
typedef enum dmProtocol_command{
  
  /*
    Newly created messages are initialized with 0 as the command value.
    Command should be explicitely choosed by the requester, else an implicit and potentially wrong command code could be sent.
  */
  UNSPECIFIED = 0,
  
  /*  Client sets payload with name of file to send, then listens for server response.
      Server responds with status OK if it can store the file successfully, else sets status with an error code and payload with an error message. Then it listens for incoming file data.
      Client sends file data to server, then listens for server response.
      Server responds with status OK if file was stored successfully, else sets status with an error code and payload with an error message.
  */
  PUT,
  
  /*  Client requests server to send file with the name specified in payload, then listens for server response.
      Server responds with status OK if file exists and it proceeds to send file data, else sets status with an error code and sets payload with an error message. 

  */
  GET,
    
  /*  Client requests server to send file list of files currently stored in server, with the file names separated with the character specified in payload.
      Server responds with status OK and fileList string in payload.
  */
  LIST,

  /*  Client requests server to allocate resources to handle one client request.
      Server responds with status OK and the port number where the client has to send its request in payload if it succesfully allocated resources to handle the client request, else sets status with an error code and payload with an error message.
  */               
  HS,

  COMMANDS_NUM
}DmProtocol_command;

// code defining status of operation after it was performed on server. If status is an error, payload is set as a string describing the error, unless something different is specified in the status 
typedef enum opStatus{
  
  /**
  * Operation failed beacuse a generic error occurred. This should be used only for internal purposes. Always try to send to dest more meaningful errors
  */
  OP_STATUS_E = -INT_MAX,           

  /**
   * Op failed beacuse a file needed by the operation was not found.
  */
  OP_STATUS_E_FILE_NOT_FOUND,

  /**
   * Operation was performed succesfully
   *
 */ 
  OP_STATUS_OK = 0,                 

} OpStatus;


/*
  divides a message into an array and sendend to the sendMessageGbn
  @param sd socket used to send packets
  @param dest_addr destination address
  @param dest_addr_size dest addr size
  @param msg pointer to message to send
  @return 0 if success, else -1
*/
int sendMessageDMProtocol(int socket, struct sockaddr *dest_addr,
  socklen_t dest_addr_size, Message *msg );

/*
  returns if we recived a message this is called
  @param sd socket used to listen packets. Must be already bound
  @param senderAddr if not NULL, sender addr will be copied on *senderAddr
  @param senderAddrlen if not NULL, sender addr len is stored on *sender_addr_len
  @param msg a pointer to a Message object populated with received data will be stored in *msg
  @return 0 if success, else -1
*/
int receiveMessageDMProtocol(int socket, struct sockaddr *sender_addr,
  socklen_t *sender_addr_len, Message **msg);

/**
 * Sends a file to a internet address using dm protocol.
 * @param socket sending socket descriptor
 * @param dest_addr pointer to dest address struct
 * @param dest_addr_size size of *dest_addr
 * @param pathName path of file to send
 * @return status of operation. See OpStatus definition
*/
int sendFileDMProtocol(int socket, struct sockaddr *dest_addr, socklen_t dest_addr_size, int fileD);

/**
 * Receives an incoming file.
 * @param socket listening socket descriptor
 * @param dest_addr if not NULL, sender addr is stored in *dest_addr
 * @param dest_addr_size if not NULL, size of sender addr is stored in *dest_addr_size
 * @param filePath path where the received file will be stored
 * @return status of operation. See OpStatus definition
*/
int receiveFileDMProtocol(int socket, struct sockaddr *sender_addr, socklen_t *send_addr_size, int fileD);

#endif
