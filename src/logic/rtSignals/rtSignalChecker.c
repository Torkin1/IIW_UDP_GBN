#include "rtSignals/rtSignalsChecker.h"
#include "logger/logger.h"
#include <signal.h>

int checkRtSignal(int rtSig){

    if (rtSig <= SIGRTMIN || rtSig > SIGRTMAX){

        logMsg(E, "checkRtSignal: %d is not a real time signal\n", rtSig);
        return 1;
    }

    return 0;

}
