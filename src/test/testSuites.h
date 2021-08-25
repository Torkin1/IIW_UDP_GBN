#ifndef TESTSUITES_H_INCLUDED
#define TESTSUITES_H_INCLUDED

#include <stdlib.h>

// Compares byte-to-byte expected and actual value. If they are not equal, a signal is raised
// size parameter shall be the size of expected value
// errorMsg will be used by handler to inform that the test failed
// cleanup function will be called after handling the failure, so use it if the test function has to do some cleanup
void assertEquals(const void* expected, const void* actual, size_t size, char *errorMsg, void (*cleanup)(void));

// test functions written below can be called by the test launcher

// void test1();
// void test2();
// ...

void testSerialize();
void testSendMessageGbn();

#endif // TESTSUITES_H_INCLUDED
