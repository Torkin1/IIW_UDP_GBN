#ifndef JAMMER_H_INCLUDED
#define JAMMER_H_INCLUDED

#include <stdbool.h>

#define JAM_RATE 50 // percentage

// returns true with a rate defined in JAM_RATE
bool isJammed();

#endif // JAMMER_H_INCLUDED
