#ifndef LAUNCHER_H_INCLUDED
#define LAUNCHER_H_INCLUDED

#include <pthread.h>

#define ACK_LISTENING_PORT 32773

// Launcher must respond to all events listed below
typedef enum launcherEvent{

    NEW_PACKETS_IN_SEND_WINDOW,         // Some new packets have been added to the battery
    PACKET_TIMED_OUT,                   // A sent packet timeout expired
    SHUTDOWN,                           // Launche must be turned off

    NUM_OF_LAUNCHER_EVENTS

} LauncherEvent;

// singleton launcher
int getLauncherId(pthread_t *tid);

// notifies launcher about an event in the launchBattery. Infos about the event required by the handler can be stored in eventInfo.
int notifyLauncher(LauncherEvent event);

// turns off launcher and does cleanup
void shutdownLauncher();

#endif // LAUNCHER_H_INCLUDED
