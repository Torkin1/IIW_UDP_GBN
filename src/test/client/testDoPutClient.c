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

void testDoPutClient(){

    char *expected;
    expected = "CIAO A TUTTI!";
    char *fileExpectedName = "test";
    int fd;

    fd = open(fileExpectedName, O_CREAT | O_TRUNC | O_WRONLY);
    write(fd, expected, strlen(expected) + 1);
    close(fd);

    doCommand(PUT, &fileExpectedName);

}