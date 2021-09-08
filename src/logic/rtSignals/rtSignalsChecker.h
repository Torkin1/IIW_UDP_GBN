#ifndef RTSIGNALSCHECKER_H_INCLUDED
#define RTSIGNALSCHECKER_H_INCLUDED

#include <stdbool.h>

/*  
    Checks if a signal value is a valid real time signal value
    @param rtSig signal value to check
    @return 0 if rtSig corresponds to a valid real time signal, else 1

*/
int checkRtSignal(int rtSig);

#endif // RTSIGNALSCHECKER_H_INCLUDED
