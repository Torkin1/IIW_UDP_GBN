#ifndef TESTSUITES_H_INCLUDED
#define TESTSUITES_H_INCLUDED

#include <stdlib.h>

#define EXIT_ON_FAIL true
#define SIGASSFLD SIGUSR1

/*
Compares byte-to-byte expected and actual value. If they are not equal, SIGASSFLD is raised and process could exit depending on EXIT_ON_FAIL value
@param expected pointer to expected value
@param actual pointer to actual value
@param size size of expected value
@param errorMsg error string message printed if assertion failed
@param cleanup function called after test failed and executed by signal handler. This ensures that cleanup actions are still performed if signal handler decides to exit process at assertion failure
*/
void assertEquals(const void* expected, const void* actual, size_t size, char *errorMsg, void (*cleanup)(void));

// test functions written below can be called by the test launcher

// void test1();
// void test2();
// ...

void testSerialize();
void testSendMessageGbn();
void testSendMessageGbnMultipleSends();
void testRecvMessageGbn();
void testRecvMessageGbnFirstPacketOutOfOrder();
void testTimerConst();
void testSendMessageDMProtocol();   // FIXME: missing implementation
void testDoHandShakeClient();
void testDoHandshakeServer();
void testParseCommandName();
void testDoListClient();
void testSendRecvFile();
void testRecvFile();
void testDoGetServer();
void testDoGetClient();

#endif // TESTSUITES_H_INCLUDED
