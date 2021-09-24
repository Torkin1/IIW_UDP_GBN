#include "../testSuites.h"
#include "client/controller.h"
#include <unistd.h>
#include <netinet/ip.h>
#include <string.h>
#include "logger/logger.h"
#include <signal.h>

void testDoListClient(){
        
        // client
        
        int expected = 0, actual;
        const char* errMsg = "doList returned an error\n";
        actual = doCommand(LIST, NULL);

        assertEquals(&expected, &actual, sizeof(int), errMsg, NULL);          
        

}