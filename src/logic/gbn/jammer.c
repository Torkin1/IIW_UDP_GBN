#include "gbn/jammer.h"
#include "logger/logger.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define JAM_RATE 50

static bool isSeeded = false;

bool isJammed(){

    /*

        NOTE:   no seed is set, perfect uniformity is not assured
                and no measure has been taken for assuring that
                different threads generate different random numbers,
                since true randomness is not a priority.
    */

    if (!isSeeded){
        srand(time(NULL));
        isSeeded = true;
    }
    
    int r = rand() % 100;
    bool res = r < JAM_RATE;
    return res;

}




