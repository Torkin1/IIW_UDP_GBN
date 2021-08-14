#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#define MSG_MAX_LEN 1024
#define LOG_LEVEL 3

enum Tag {I, E, W, D};

void logMsg(enum Tag tag, const char* msg, ...);

#endif // LOGGER_H_INCLUDED
