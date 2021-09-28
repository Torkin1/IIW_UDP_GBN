#include "../testSuites.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "dm_protocol/dm_protocol.h"
#include <netinet/ip.h>
#include "logger/logger.h"
#include <errno.h>

void testSendRecvFile(){

    //FIXME: ordine dei messaggi non è rispettato, se arrivano messaggi fuori ordine quelli vecchi sono scartati
    
    const char *expected = "CIAO A TUTTI";
    const char *TEST_FILE_NAME = "test";
    char actual[1024] = {0};
    const in_port_t LISTENING_PORT = 8888; 
    int fd, sd, fdRcvd;
    struct sockaddr_in destAddr = {0};
    char errMsg[2048] = {0};
    
    fd = open(TEST_FILE_NAME, O_CREAT | O_APPEND | O_TRUNC | O_RDWR);
    write(fd, expected, strlen(expected));
    close(fd);
    
    logMsg(I, "expected output written in file\n");

    destAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    destAddr.sin_port = htons(LISTENING_PORT);
    destAddr.sin_family = AF_INET;

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    logMsg(D, "BURP!\n");
    sendFileDMProtocol(sd, (struct sockaddr *) &destAddr, sizeof (struct sockaddr_in), TEST_FILE_NAME);
    logMsg(I, "requested to send file\n");
    fdRcvd = open("testRcvd", O_RDONLY | O_NONBLOCK);
    lseek(fdRcvd, 0, SEEK_SET);
    read(fdRcvd, actual, strlen(expected));
    close(fdRcvd);
    logMsg(I, "actual output read from file\n");

    sprintf(errMsg, "testSendRecvFile: expected: %s, actual: %s\n", expected, actual);
    assertEquals(expected, actual, strlen(expected), errMsg, NULL);

    
    
}