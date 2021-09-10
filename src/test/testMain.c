#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "testSuites.h"
#include "logger/logger.h"

typedef struct assertionFailureInfo{

    char *msg;
    void (*onFailure) (void);

} AssertionFailureInfo;
//
void assertEquals(const void* expected, const void* actual, size_t size, char *errorMsg, void (*onFailure)(void)){

    // compares bytes of values
    if (memcmp(expected, actual, size)){

        // raises failure of assertion
        AssertionFailureInfo *info = calloc(1, sizeof(AssertionFailureInfo));
        info ->msg = errorMsg;
        info ->onFailure = onFailure;

        union sigval val = {0};
        val.sival_ptr = info;

        pthread_sigqueue(pthread_self(), SIGASSFLD, val);
        free(info);
    }

}

void assertionFailureHandler(int sig, siginfo_t *siginfo, void *ucontext){

    // prints failure info
    AssertionFailureInfo *info = siginfo -> si_ptr;
    logMsg(I, "Test failed. Message: %s\n", info -> msg);

    // does cleanup instructed by test function
    void (*onFailure)(void) = ((AssertionFailureInfo *)(siginfo ->si_ptr)) -> onFailure;
    if (onFailure != NULL){
        onFailure();
    }

    // cleanup
    if (EXIT_ON_FAIL){
        exit(EXIT_FAILURE);
    }
}

static struct sigaction assertionFailureHandlerAct;

int main(){

    // sets up handler for failing assertion
    struct sigaction oldAct;
    assertionFailureHandlerAct.sa_flags = SA_SIGINFO;
    assertionFailureHandlerAct.sa_sigaction = assertionFailureHandler;

    if (sigaction(SIGASSFLD, &assertionFailureHandlerAct, &oldAct)){

        int err = errno;
        logMsg(E, "%s\n", strerror(err));
    }


    // write test functions here

    // test1();
    // test2();
    // ...

    testSendMessageDMProtocol();

    logMsg(I, "\n\n*** All test are successful, great job! ***\n");

    return EXIT_SUCCESS;

}
