#include "client/controller.h"
#include "dm_protocol/dm_protocol.h"
#include "logger/logger.h"
#include <errno.h>
#include "../testSuites.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void testDoGetClient(){

    char *expected, *actual;
    expected = "CIAO A TUTTI!";
    char *fileExpectedName = "test";
    int fd;
    char errorMsg[1024];

    doCommand(GET, &fileExpectedName);

    fd = open(fileExpectedName, O_RDONLY);
    actual = mmap(NULL, strlen(expected) + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (actual == MAP_FAILED){
        int err = errno;
        logMsg(E, "mmap failed: %s\n", strerror(err));
    }
    close(fd);

    sprintf(errorMsg, "testDoGetClient: expected %s, actual %s\n", expected, actual);
    assertEquals(expected, actual, strlen(expected) + 1, errorMsg, NULL);
}