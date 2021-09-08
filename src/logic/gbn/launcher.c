#define _GNU_SOURCE

#include "gbn/launchBattery.h"
#include <stdbool.h>
#include <signal.h>
#include "logger/logger.h"
#include "rtSignals/rtSignalsChecker.h"
#include <string.h>
#include "gbn/launcher.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "gbn/packet.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include "gbn/jammer.h"
#include "gbn/timer.h"
#include "gbn/networkSampler.h"

// launcher control thread data
static pthread_t launcherId;
static bool isLauncherAvailable;
static pthread_cond_t isLauncherAvailableCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t launcherInitializerLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t isLauncherInitialized = PTHREAD_ONCE_INIT;

// launcher event handler structs
static struct sigaction launcherEventsAct, oldAct;
static bool isOldActSet = false;

// socket ack listener
static int listenAckSocket = -1;

// singleton timer for oldest packet in window on air
static Timer *timer;

Timer *getTimerReference(){

    if (timer == NULL){
        timer = newTimer();
    }
    return timer;
}

// gets the real time signal value corresponding to the provided launcher event
int getSignalFromLauncherEvent(LauncherEvent event)
{

    int signal = SIGRTMIN + (int) event + 1;
    if (checkRtSignal(signal))
    {
        logMsg(E, "getSignalFromLauncherEvent: %d is not a valid launcher event\n", event);
        return -1;
    }
    return signal;

}

void launcherCleanup()
{

    // restores original signal disposition
    if (isOldActSet)
    {
        for (LauncherEvent e = 0; e < NUM_OF_LAUNCHER_EVENTS; e ++)
        {
            if (sigaction(getSignalFromLauncherEvent(e), (isOldActSet)? NULL : &oldAct, NULL))
            {
                int err = errno;
                logMsg(W, "launcherCleanup: unable to restore original launcher event handler: %s\n", strerror(err));
            }
        }
    }

    // destroys timer for oldest packet in window
    if (getTimerReference() ->isAlive){
        timeout(getTimerReference(), AT_TIMEOUT_SHUTDOWN);
        pthread_join(getTimerReference() ->timerTid, NULL);
    }  
    destroyTimer(getTimerReference());
    
    // closes ack listen socket
    close(listenAckSocket);

}

int fireLaunchPad(LaunchPad *currentPad)
{

    uint8_t *sendbuf;
    int sendbufSize, err;
    struct sockaddr_in ackAddr;
    socklen_t ackAddrLen = sizeof(struct sockaddr_in);

    // writes in packet port number where the launcher will listen for acks
    if (getsockname(listenAckSocket, &ackAddr, &ackAddrLen)){
        err = errno;
        logMsg(E, "fireLaunchPad: unable to get ack socket address: %s\n", strerror(err));
        return -1;
    }
    currentPad ->packet ->header ->ackPort = ntohs(ackAddr.sin_port);

    // serializes packet
    sendbuf = serializePacket(currentPad ->packet);
    sendbufSize = calcPacketSize(currentPad ->packet);

    // dest addr can be retrieved in send table
    pthread_mutex_lock(&sendTableLock);
    SortingEntry *currentSendEntry = getFromSortingTable(getSendTableReference(), currentPad ->packet ->header ->msgId);
    
    // packet shall be discarded with a probability of p. This is to simulate a loss event
    if (!isJammed())
    {
        // sends serialized packet
        // TODO: measure time needed for send to successfully return and update the upload time with that value.    
        if ((err = sendto(currentSendEntry ->sd, sendbuf, sendbufSize, MSG_NOSIGNAL, currentSendEntry ->addr, currentSendEntry ->addrlen)) < 0)
        {

            err = errno;
            logMsg(E, "fireLaunchPad: sendto failed: %s\n", strerror(err));
            pthread_mutex_unlock(&sendTableLock);
            return -1;

        }
        logMsg(D, "fireLaunchPad: launched packet with index %d\n", currentPad ->packet ->header ->index);

    } else {

        logMsg(D, "fireLaunchPad: launchPad %d is jammed!\n", currentPad ->packet ->header ->index);
    }

    pthread_mutex_unlock(&sendTableLock);

    // updates launch pad stats
    currentPad ->status = SENT;
    currentPad ->launches ++;
    clock_gettime(CLOCK_REALTIME, &(currentPad ->launchTime));

    free(sendbuf);

    return 0;

}

int sendAllPacketsInWindowCore(LaunchPadStatus statuses[], int numOfStatuses)
{

    LaunchBattery *battery = getLaunchBatteryReference();

    int base, nextSeqNum;
    SendWindow *window = getSendWindowReference();

    base = window -> base;
    nextSeqNum = window -> nextSeqNum;

    LaunchPad *currentPad;

    // this handles the case when nextSeqNum < base  (battery has a circular structure)
    if (nextSeqNum < base)
    {
        nextSeqNum += QUEUE_LEN;
    }
    int launches = 0;
    for (int i = base; i < nextSeqNum; i ++)
    {
        currentPad = (battery -> battery)[i % QUEUE_LEN];
        for (int j = 0; j < numOfStatuses; j++ )
        {
            if (currentPad -> status == statuses[j])
            {
                if (fireLaunchPad(currentPad) >= 0){
                    launches ++;
                }
                break;
            }
        }
    }

    return launches;

}

// sends all READY and SENT packets in send window
int sendAllPacketsInWindow()
{

    LaunchPadStatus statuses[2];
    statuses[0] = READY;
    statuses[1] = SENT;
    return sendAllPacketsInWindowCore(statuses, 2);

}

int sendAllReadyPacketsInWindow()
{

    LaunchPadStatus status = READY;
    return sendAllPacketsInWindowCore(&status, 1);

}

int sendAllSentPacketsInWindow(){

    LaunchPadStatus status = SENT;
    return sendAllPacketsInWindowCore(&status, 1);
}

void ring(){

    notifyLauncher(LAUNCHER_EVENT_PACKET_TIMED_OUT);
}

// handles launcher events
void launcherSigHandler(int sig, siginfo_t *siginfo, void *ucontext)
{

    logMsg(D, "launcherSigHandler: captured signal %d\n", sig);
    
    // can't use a switch because real time signal value are determined at run time
    if (sig == getSignalFromLauncherEvent(LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW))
    {
        int launches;

        // sends all ready packets in send window
        pthread_mutex_lock(&launchBatteryLock);
        pthread_mutex_lock(&sendWindowLock);

        launches = sendAllReadyPacketsInWindow();

        pthread_mutex_unlock(&sendWindowLock);
        pthread_mutex_unlock(&launchBatteryLock);

        logMsg(D, "launcherSigHandler: %d READY packets sent\n", launches);

        // (re)starts timeout for the oldest packet in window if not already started
        
        if (launches > 0){
            if (!(getTimerReference()->isAlive))
            {
                startTimer(getTimerReference(), AT_TIMEOUT_RING_THEN_RESTART, ring);
            }
        }

    }
    else if (sig == getSignalFromLauncherEvent(LAUNCHER_EVENT_PACKET_TIMED_OUT))
    {
        int launches = 0;
        
        // sends all sent packets in send window
        pthread_mutex_lock(&launchBatteryLock);
        pthread_mutex_lock(&sendWindowLock);

        launches = sendAllSentPacketsInWindow();
        logMsg(D, "launcherSigHandler: %d SENT packets resent\n",launches);

        pthread_mutex_unlock(&sendWindowLock);
        pthread_mutex_unlock(&launchBatteryLock);

        // restarts timer
        if (launches == 0 && getTimerReference()->isAlive)
        {
            timeout(getTimerReference(), AT_TIMEOUT_SHUTDOWN);
            pthread_join(getTimerReference() ->timerTid, NULL);
        }

    }

    else if (sig == getSignalFromLauncherEvent(LAUNCHER_EVENT_SHUTDOWN))
    {

        isLauncherAvailable = false;
    }

    else
    {
        logMsg(W, "launcherSigHandler: registered signal %d but don't know what to do with it\n", sig);
    }

}

void listenForAcks()
{

    void *buf = calloc(1, calcAckSize());
    struct sockaddr_in fromWho;
    socklen_t fromWhoLen = sizeof(struct sockaddr_in);
    int recvfromRes, base, ackIndex, newBase, newNextSeqNum, ackedPacks;
    Packet *ack;
    LaunchPad *currentPad;
    struct timespec now;
    struct timespec sampleRtt = {0};
    sigset_t blockedSignalsWhileAcking;
    
    // all launcher events will be blocked while acking packets (but not when listening for acks)
    sigemptyset(&blockedSignalsWhileAcking);
    for (LauncherEvent e = 0; e < NUM_OF_LAUNCHER_EVENTS; e ++){
        sigaddset(&blockedSignalsWhileAcking, getSignalFromLauncherEvent(e));
    }

    // launcher listens for acks for all its life
    while (isLauncherAvailable)
    {

        if (recvfrom(listenAckSocket, buf, calcAckSize(), 0, &fromWho, &fromWhoLen) < 0)
        {

            recvfromRes = errno;

            if (recvfromRes == EINTR)
            {

                // probably we have just sent all the packets in the window. We restart to listen for acks
                continue;
            }
            else
            {

                logMsg(E, "listenForAcks: an error occcurred while listening for acks: %s\n", strerror(errno));
            }
        }

        else
        {

            // a packet has been received, but there is no guarantee that it is an ACK
            ack = deserializePacket(buf);
            if (ack->header->isAck)
            {

                pthread_sigmask(SIG_BLOCK, &blockedSignalsWhileAcking, NULL);
                pthread_mutex_lock(&sendWindowLock);

                base = getSendWindowReference()->base;

                // acks referred to already acked packets are discarded
                ackIndex = ack->header->index;
                logMsg(D, "listenForAcks: accepted ACK with msgId=%d and index=%d\n", ack->header->msgId, ackIndex);

                // all packets up to the specified index are ACKED
                pthread_mutex_lock(&launchBatteryLock);

                if (ackIndex < base)
                {
                    ackIndex += QUEUE_LEN;
                }
                ackedPacks = 0;
                for (int i = base; i < ackIndex; i++)
                {
                    currentPad = getLaunchBatteryReference()->battery[i % QUEUE_LEN];
                    if (currentPad->status != ACKED)
                    {
                        currentPad->status = ACKED;
                        updateContiguousPads(1);
                        ackedPacks++;

                        // calculates rtt sample and updates estimated rtt
                        clock_gettime(CLOCK_REALTIME, &now);
                        sampleRtt.tv_sec = now.tv_sec - currentPad ->launchTime.tv_sec;
                        sampleRtt.tv_nsec = now.tv_nsec - currentPad ->launchTime.tv_nsec;
                        pthread_mutex_lock(&networkSamplerLock);
                        updateEstimatedRtt(sampleRtt);
                        pthread_mutex_unlock(&networkSamplerLock);
                        
                    }
                }

                // If ACK was in order at least one packet has been acked
                if (ackedPacks > 0)
                {
                    // if it's the last packet of the message we do some cleanup
                    if (ackIndex == ((ack->header->endIndex) + 1) % QUEUE_LEN)
                    {
                        logMsg(D, "listenForAcks: message %d fully sent\n", ack->header->msgId);

                        // remove the corresponding entry from the send table
                        pthread_mutex_lock(&sendTableLock);
                        removeFromSortingTable(getSendTableReference(), ack->header->msgId);
                        pthread_mutex_unlock(&sendTableLock);
                    }

                    // timeout of oldest packet in window must be stopped
                    if (getTimerReference()->isAlive)
                    {
                        timeout(getTimerReference(), AT_TIMEOUT_RESTART);
                    }

                    // updates send window
                    newBase = ackIndex % QUEUE_LEN;
                    newNextSeqNum = (newBase + getWinSize()) % QUEUE_LEN;
                    getSendWindowReference()->base = newBase;
                    getSendWindowReference()->nextSeqNum = newNextSeqNum;
                    logMsg(D, "listenForAcks: updated send window base: %d, nextSeqNum: %d\n", getSendWindowReference()->base, getSendWindowReference()->nextSeqNum);
                }

                pthread_mutex_unlock(&launchBatteryLock);
                pthread_mutex_unlock(&sendWindowLock);

                // send window could now contain packets to send;
                notifyLauncher(LAUNCHER_EVENT_NEW_PACKETS_IN_SEND_WINDOW);
                pthread_sigmask(SIG_UNBLOCK, &blockedSignalsWhileAcking, NULL);

                destroyPacket(ack);
            }
        }
    }
}

// launcher routine
void *launcher(void *args)
{

    logMsg(D, "launcher: launcher is alive\n");

    pthread_mutex_lock(&launcherInitializerLock);

    // sets up listen socket for acks
    listenAckSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = 0;  // port number is decided at runtime among free user ports
    bind(listenAckSocket, &addr, sizeof(struct sockaddr_in));

    // we let others know that the launcher thread is running
    isLauncherAvailable = true;
    pthread_mutex_unlock(&launcherInitializerLock);
    pthread_cond_signal(&isLauncherAvailableCond);

    // listens for incoming acks
    listenForAcks();

    // cleanup
    logMsg(D, "launcher: I'm dying lol\n");
    launcherCleanup();
    return NULL;

}


// launcher singleton implementation
void initLauncher()
{

    int err;

    // sets up handler for launcher events
    launcherEventsAct.sa_flags = SA_SIGINFO;
    launcherEventsAct.sa_sigaction = launcherSigHandler;    
    for (LauncherEvent e = 0; e < NUM_OF_LAUNCHER_EVENTS; e ++)
    {
        sigemptyset(&launcherEventsAct.sa_mask);
        for (LauncherEvent be = 0; be < NUM_OF_LAUNCHER_EVENTS; be++)
        {
            if (be != e){
                sigaddset(&launcherEventsAct.sa_mask, getSignalFromLauncherEvent(be));
            }
        }

        if (sigaction(getSignalFromLauncherEvent(e), &launcherEventsAct, (isOldActSet)? NULL : &oldAct))
        {
            int err = errno;
            logMsg(E, "initLauncher: unable to set launcher event handler: %s\n", strerror(err));
            return;
        }
        if (!isOldActSet)
        {
            isOldActSet = true;
        }
    }

    
    // starts launcher
    pthread_mutex_lock(&launcherInitializerLock);
    if ((err = pthread_create(&launcherId, NULL, launcher, NULL)))
    {
        logMsg(E, "initLauncher: cannot create launcher thread: %s\n", strerror(err));
        return;
    }

    // waits for launcher to be fully started
    while(!isLauncherAvailable)
    {
        pthread_cond_wait(&isLauncherAvailableCond, &launcherInitializerLock);
    }
    pthread_mutex_unlock(&launcherInitializerLock);

    logMsg(D, "initLauncher: launcher created: tid = %d\n", launcherId);
}


int getLauncherId(pthread_t *tid)
{

    if (!isLauncherAvailable)
    {

        // synchronized
        pthread_once(&isLauncherInitialized, initLauncher);

        // if launcher tid is still not available, something went wrong in initLauncher
        if (!isLauncherAvailable)
        {
            logMsg(E, "getLauncherId: failed to start launcher thread\n");
            return -1;
        }
    }

    // all ok, we can finally give launcher tid to the caller
    if (tid != NULL)
    {
        *tid = launcherId;
    }

    return 0;

}


// notifies launcher about an event. eventInfo must be allocated dynamically
int notifyLauncher(LauncherEvent event)
{

    int err;
    pthread_t launcherId;

    // launcher could be not available
    if (getLauncherId(&launcherId))
    {
        logMsg(E, "notifyLauncher: launcher not available\n");
        return -1;
    }

    logMsg(D, "notifyLauncher: requested to notify launcher of event %d\n", event);

    if ((err = pthread_kill(launcherId, getSignalFromLauncherEvent(event))))
    {
        logMsg(E, "notifyLauncher: failed to send signal to launcher: %s\n", strerror(err));
        return -1;
    }

    return 0;

}







