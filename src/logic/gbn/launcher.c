#define _GNU_SOURCE

#include "launchBattery.h"
#include <stdbool.h>
#include <signal.h>
#include "logger/logger.h"
#include "rtSignals/rtSignalsChecker.h"
#include <string.h>
#include "launcher.h"
#include <errno.h>
#include <unistd.h>
#include "sendTable.h"
#include <stdlib.h>
#include <pthread.h>
#include "packet.h"
#include "sendWindow.h"

// singleton
static pthread_t launcherId;
static bool isLauncherAvailable;
static pthread_cond_t isLauncherAvailableCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t launcherInitializerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t isLauncherInitialized = PTHREAD_ONCE_INIT;

// launcher event handler structs
static struct sigaction launcherEventsAct, oldAct;
bool isOldActSet = false;

// gets the real time signal value corresponding to the provided launcher event
int getLauncherSignal(LauncherEvent event){

    int signal = SIGRTMIN + (int) event + 1;
    if (checkRtSignal(signal)){
        logMsg(E, "getSignalFromLauncherEvent: %d is not a valid launcher event\n", event);
        return -1;
    }
    return signal;

}

void launcherCleanup(){

    int err;

    // restores original signal disposition
    if (isOldActSet){
        for (LauncherEvent e = 0; e < NUM_OF_LAUNCHER_EVENTS; e ++){
            if (sigaction(getLauncherSignal(e), (isOldActSet)? NULL : &oldAct, NULL)){
                int err = errno;
                logMsg(W, "launcherCleanup: unable to restore original launcher event handler: %s\n", strerror(err));
            }
        }
    }

}

void fireLaunchPad(LaunchPad *currentPad){

    uint8_t *sendbuf;
    int sendbufSize;
    int err;

    // dest addr can be retrieved in sorting table
    SendEntry *currentSendEntry = getFromSendTable(getSendTableReference(), currentPad ->packet ->header ->msgId);

    // TODO: packet shall be discarded with a probability of p. This is to simulate a loss event

    // serializes packet
    sendbuf = serializePacket(currentPad ->packet);
    sendbufSize = calcPacketSize(currentPad ->packet);

    // TODO: measure time needed for send to successfully return and update the upload time with that value.

    // sends serialized packet
    if ((err = sendto(currentSendEntry ->sd, sendbuf, sendbufSize, MSG_NOSIGNAL, currentSendEntry ->dest_addr, currentSendEntry ->addrlen)) < 0){

        err = errno;
        logMsg(E, "sendAllPacketsInWindow: sendto failed: %s\n", strerror(err));

    }

    // updates launch pad data
    currentPad ->status = SENT;
    currentPad ->launches ++;

    free(sendbuf);

}


// sends all packets in send window
int sendAllPacketsInWindow(){

    int err;

    // locks battery
    LaunchBattery *battery = getLaunchBatteryReference();
    if ((err = pthread_mutex_lock(&(battery ->lock)))){
        logMsg(E, "sendAllPacketsInWindows: can't get lock on launch battery: %s\n", strerror(err));
        return -1;
    }

    // locks window and stores current values
    int base, nextSeqNum;
    SendWindow *window = getSendWindowReference();
    if ((err = pthread_mutex_lock(&(window ->lock)))){
        logMsg(E, "sendAllPacketsInWindows: can't get lock on send window: %s\n", strerror(err));
        return -1;
    }

    base = window -> base;
    nextSeqNum = window -> nextSeqNum;

    if ((err = pthread_mutex_unlock(&(window ->lock)))){
        logMsg(E, "sendAllPacketsInWindows: can't release lock on send window: %s\n", strerror(err));
        return -1;
    }


    // sends all READY packets in send window and updates their launch counter
    LaunchPad *currentPad;

    for (int i = base; i < nextSeqNum; i ++){
        currentPad = (battery -> battery)[i];
        if(currentPad ->status == READY || currentPad -> status == SENT){

            fireLaunchPad(currentPad);
        }
    }

    if ((err = pthread_mutex_unlock(&(battery ->lock)))){
        logMsg(E, "sendAllPacketsInWindows: can't release lock on launch battery: %s\n", strerror(err));
        return -1;
    }

    return 0;

}

void launcherSigHandler(int sig, siginfo_t *siginfo, void *ucontext){

    // can't use a switch because real time signal value are determined at run time
    if (sig == getLauncherSignal(NEW_PACKETS_IN_SEND_WINDOW)){

        sendAllPacketsInWindow();
        logMsg(D, "all packets in window sent\n");

        // TODO: sets timeout for the oldest packet in window
    }
    else if (sig == getLauncherSignal(PACKET_TIMED_OUT)){

        // TODO:
    }

    else if (sig == getLauncherSignal(PACKET_ACKED)){

        // TODO:
    }

    else if (sig == getLauncherSignal(SHUTDOWN)){

        isLauncherAvailable = false;
    }

    else {
        logMsg(W, "launcherSigHandler: registered signal %d but don't know what to do with it\n", sig);
    }

}

// launcher routine
void *launcher(void *args){

    logMsg(D, "launcher is alive\n");

    pthread_mutex_lock(&launcherInitializerLock);

    // no need to join this thread
    pthread_detach(pthread_self());

    // before waiting, we let others know that the launcher thread is running
    isLauncherAvailable = true;
    pthread_mutex_unlock(&launcherInitializerLock);
    pthread_cond_signal(&isLauncherAvailableCond);

    // waits for some event to happen
    while(isLauncherAvailable){
        logMsg(D, "launcher is waiting for events ...\n");
        pause();
    }

    launcherCleanup();
    return NULL;

}


// launcher singleton implementation
void initLauncher(){

    int err;

    // sets up handler for launcher events
    launcherEventsAct.sa_flags = SA_SIGINFO;
    launcherEventsAct.sa_sigaction = launcherSigHandler;

    for (LauncherEvent e = 0; e < NUM_OF_LAUNCHER_EVENTS; e ++){
        if (sigaction(getLauncherSignal(e), &launcherEventsAct, (isOldActSet)? NULL : &oldAct)){
            int err = errno;
            logMsg(E, "initLauncher: unable to set launcher event handler: %s\n", strerror(err));
            return;
        }
        if (!isOldActSet){
            isOldActSet = true;
        }
    }

    // starts launcher
    pthread_mutex_lock(&launcherInitializerLock);
    if ((err = pthread_create(&launcherId, NULL, launcher, NULL))){
        logMsg(E, "initLauncher: cannot create launcher thread: %s\n", strerror(err));
        return;
    }

    // waits for launcher to be fully started
    while(!isLauncherAvailable){
        pthread_cond_wait(&isLauncherAvailableCond, &launcherInitializerLock);
    }
    pthread_mutex_unlock(&launcherInitializerLock);

    logMsg(D, "launcher created: tid = %d\n", launcherId);
}


// sd must be specified only the first time. Subsequent calls to this function will ignore sd
int getLauncherId(pthread_t *tid){

    if (!isLauncherAvailable){

        // synchronized
        pthread_once(&isLauncherInitialized, initLauncher);

        // if launcher tid is still not available, something went wrong in initLauncher
        if (!isLauncherAvailable){
            logMsg(E, "getLauncherId: failed to start launcher thread\n");
            return -1;
        }
    }

    // all ok, we can finally give launcher tid to the caller
    *tid = launcherId;

    return 0;

}


// notifies launcher about an event. eventInfo must be allocated dynamically
int notifyLauncher(LauncherEvent event, void *eventInfo){

    int err;
    pthread_t launcherId;

    // launcher could be not available
    if (getLauncherId(&launcherId)){
        logMsg(E, "notifyLauncher: launcher not available\n");
        return -1;
    }

    // notifies launcher
    union sigval value = {0};
    value.sival_ptr = eventInfo;

    if ((err = pthread_sigqueue(launcherId, getLauncherSignal(event), value))){
        logMsg(E, "notifyLauncher: failed to queue signal to launcher: %s\n", strerror(err));
        return -1;
    }

    return 0;

}







