#ifndef LAUNCHER_H_INCLUDED
#define LAUNCHER_H_INCLUDED

#include <pthread.h>

#define ACK_LISTENING_PORT 32773

// Launcher must respond to all events listed below
typedef enum launcherEvent{

    LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW,         // Some new packets have been added to the battery
    LAUNCHER_EVENT_PACKET_TIMED_OUT,                   // A sent packet timeout expired
    LAUNCHER_EVENT_SHUTDOWN,                           // Launcher must be turned off

    NUM_OF_LAUNCHER_EVENTS

} LauncherEvent;

// singleton launcher
int getLauncherId(pthread_t *tid);

// notifies launcher about an event.
int notifyLauncher(LauncherEvent event);

// turns off launcher and does cleanup
void shutdownLauncher();

#endif // LAUNCHER_H_INCLUDED
