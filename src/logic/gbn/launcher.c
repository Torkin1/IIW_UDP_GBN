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
#include "gbn/sendTable.h"
#include <stdlib.h>
#include <pthread.h>
#include "gbn/packet.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include "gbn/jammer.h"

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

// index in the battery of the last packed acked
static int lastIndexAcked = -1;

// gets the real time signal value corresponding to the provided launcher event
int getLauncherSignal(LauncherEvent event)
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
            if (sigaction(getLauncherSignal(e), (isOldActSet)? NULL : &oldAct, NULL))
            {
                int err = errno;
                logMsg(W, "launcherCleanup: unable to restore original launcher event handler: %s\n", strerror(err));
            }
        }
    }

    close(listenAckSocket);

}

int fireLaunchPad(LaunchPad *currentPad)
{

    uint8_t *sendbuf;
    int sendbufSize, err;
    struct sockaddr_in ackAddr;
    socklen_t ackAddrLen = sizeof(struct sockaddr_in);

    // dest addr can be retrieved in send table
    SendEntry *currentSendEntry = getFromSendTable(getSendTableReference(), currentPad ->packet ->header ->msgId);

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

    // TODO: measure time needed for send to successfully return and update the upload time with that value.

    // packet shall be discarded with a probability of p. This is to simulate a loss event
    if (!isJammed())
    {
        // sends serialized packet
        if ((err = sendto(currentSendEntry ->sd, sendbuf, sendbufSize, MSG_NOSIGNAL, currentSendEntry ->dest_addr, currentSendEntry ->addrlen)) < 0)
        {

            err = errno;
            logMsg(E, "fireLaunchPad: sendto failed: %s\n", strerror(err));
            return -1;

        }
        logMsg(D, "fireLaunchPad: launched packet with index %d\n", currentPad ->packet ->header ->index);

    } else {

        logMsg(W, "fireLaunchPad: launchPad %d is jammed!\n", currentPad ->packet ->header ->index);
    }

    // updates launch pad stats
    currentPad ->status = SENT;
    currentPad ->launches ++;

    free(sendbuf);

    return 0;

}

int sendAllPacketsInWindowCore(LaunchPadStatus statuses[], int numOfStatuses)
{

    // locks battery
    LaunchBattery *battery = getLaunchBatteryReference();

    // locks window and stores current values
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
    for (int i = base; i < nextSeqNum; i ++)
    {
        currentPad = (battery -> battery)[i % QUEUE_LEN];
        for (int j = 0; j < numOfStatuses; j++ )
        {
            if (currentPad -> status == statuses[j])
            {
                fireLaunchPad(currentPad);
                break;
            }
        }
    }

    return 0;

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

void launcherSigHandler(int sig, siginfo_t *siginfo, void *ucontext)
{

    // can't use a switch because real time signal value are determined at run time
    if (sig == getLauncherSignal(NEW_PACKETS_IN_SEND_WINDOW))
    {

        LaunchBattery *battery = getLaunchBatteryReference();
        SendWindow *window = getSendWindowReference();

        // sends all ready packets in send window
        pthread_mutex_lock(&(battery ->lock));
        pthread_mutex_lock(&(window ->lock));

        sendAllReadyPacketsInWindow();

        pthread_mutex_unlock(&(window ->lock));
        pthread_mutex_unlock(&(battery ->lock));

        logMsg(D, "launcherSigHandler: all ready packets in window sent\n");

        // TODO: sets timeout for the oldest packet in window
    }
    else if (sig == getLauncherSignal(PACKET_TIMED_OUT))
    {

        // TODO:
    }

    else if (sig == getLauncherSignal(SHUTDOWN))
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
    int recvfromRes, base, ackedIndex, newBase;
    Packet *ack;
    LaunchPad *currentPad;

    // launcher listens for acks for all its life
    while(isLauncherAvailable)
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
            if (ack ->header ->isAck)
            {

                pthread_mutex_lock(&(getSendWindowReference() ->lock));

                base = getSendWindowReference() ->base;

                // acks referred to already acked packets are discarded
                ackedIndex = ack ->header ->index;
                bool isIndexOutOfOrder = (ackedIndex <= lastIndexAcked) && ((lastIndexAcked - ackedIndex) <= getWinSize());
                if (isIndexOutOfOrder)
                {
                    logMsg(D, "listenForAcks: discarded ack out of order\n");

                }
                else
                {
                    logMsg(D, "listenForAcks: accepted ACK with msgId=%d and index=%d\n", ack -> header ->msgId, ackedIndex);

                    // all packets up to the specified index are ACKED
                    pthread_mutex_lock(&(getLaunchBatteryReference() ->lock));

                    if (ackedIndex < base)
                    {
                        ackedIndex += QUEUE_LEN;
                    }
                    for (int i = base; i <= ackedIndex; i ++)
                    {
                        currentPad = getLaunchBatteryReference() ->battery[i % QUEUE_LEN];
                        currentPad ->status = ACKED;
                        updateContiguousPads(1);

                    }
                    lastIndexAcked = ackedIndex;

                    // if it's the last packet of the message we remove the corresponding entry from the send table
                    if (ackedIndex == ack -> header ->endIndex)
                    {
                        logMsg(D, "listenForAcks: message %d fully sent\n", ack ->header ->msgId);
                        removeFromSendTable(getSendTableReference(), ack -> header ->msgId);
                    }

                    // TODO: timeout of oldest packet in window must be reset

                    // updates send window
                    newBase = (ackedIndex + 1) % QUEUE_LEN;
                    getSendWindowReference() ->base = newBase;
                    getSendWindowReference() ->nextSeqNum = (newBase + getWinSize()) % QUEUE_LEN;

                    // send window could now contain packets to send;
                    sendAllReadyPacketsInWindow();

                    pthread_mutex_unlock(&(getLaunchBatteryReference() ->lock));
                }

                destroyPacket(ack);
                pthread_mutex_unlock(&(getSendWindowReference() ->lock));

            }
        }

    }

}

// launcher routine
void *launcher(void *args)
{

    logMsg(D, "launcher: launcher is alive\n");

    pthread_mutex_lock(&launcherInitializerLock);

    // no need to join this thread
    pthread_detach(pthread_self());


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
        if (sigaction(getLauncherSignal(e), &launcherEventsAct, (isOldActSet)? NULL : &oldAct))
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

    if ((err = pthread_kill(launcherId, getLauncherSignal(event))))
    {
        logMsg(E, "notifyLauncher: failed to send signal to launcher: %s\n", strerror(err));
        return -1;
    }

    return 0;

}







