#include <stdlib.h>
#include "client/controller.h"
#include "logger/logger.h"
#include "dm_protocol/dm_protocol.h"

int main(int argc, char *argv[]){

  DmProtocol_command toInvoke;
  char **toInvokeArgs;
  
  if (parseCommandName(argc, argv, &toInvoke) < 0){
      logMsg(E, "main: failed to parse command name\n");
      return EXIT_FAILURE;
  }

  toInvokeArgs = argv + 2;
  if (doCommand(toInvoke, toInvokeArgs) < 0){
    logMsg(E, "main: operation failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}