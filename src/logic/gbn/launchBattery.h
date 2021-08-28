#ifndef LAUNCHBATTERY_H_INCLUDED
#define LAUNCHBATTERY_H_INCLUDED

#include <stdbool.h>
#include <pthread.h>
#include "gbn/packet.h"

// max num of packets that can occupy the queue
#define QUEUE_LEN 100   // must be much greater than sendWindow

// Each launch pad can only be in one of the following statuses
typedef enum launchPadStatus{

    EMPTY,                      // launch pad has no packet in it
    READY,                      // hosted packet is ready to be sent the first time
    SENT,                       // hosted packet has already been sent at least one time and it is waiting for ACK
    ACKED,                      // hosted packet is acked
    LOST,                       // hosted packet has no chance to be sent (e.g max rtx attempts reached)
    NUM_LAUNCHPAD_STATUSES

} LaunchPadStatus;

// Send queue can be imagined as a launch battery, where each seat is a launch pad.
// Every seat in the launch battery is a launch pad, which can hold a packet and tracks its status
typedef struct launchPad{

    Packet *packet;             // packet in the seat
    unsigned int launches;      // number of times the packet in the seat has been transmitted
    LaunchPadStatus status;     // status of launch pad

} LaunchPad;

// this is intended to be singleton
typedef struct launchBattery{

    LaunchPad *battery[QUEUE_LEN];          // the actual queue
    int contiguousPadsAvailable;            /*
                                                tracks number of contiguous pads available.
                                                This redundancy is useful because it kills the need to check every pad to know if there is enough space to add more packets.
                                                Must be updated whenever new packets are added or packets in battery are ACKED or LOST
                                            */
    pthread_mutex_t lock;                   // lock on the battery
    int nextAvailableIndex;                 // index next to the last pad occupied

} LaunchBattery;

LaunchBattery *getLaunchBatteryReference();
int destroyLaunchBattery(LaunchBattery *self);

// tells if a launch pad is available to host a new packet
bool isLaunchPadEmpty(LaunchPad *self);

// adds packets to launchbattery
int addToLaunchBatteryAtomically(Packet *packets[], int n);

// tells if there is enough space in the battery for n packets
bool willTheyFit(int n);

// call this to keep track of contiguous pads in battery
int updateContiguousPads(int change);

// window used to choose which packets are to be sent from the queue
typedef struct sendWindow{

    int base;               // last non-acked packet index in window
    int nextSeqNum;         // Index of first READY packet next to the last SENT packet in window
    pthread_mutex_t lock;

} SendWindow;

SendWindow *getSendWindowReference();
void destroySendWindow(SendWindow *self);

int getWinSize();

bool isInWindow(int i);


#endif // LAUNCHBATTERY_H_INCLUDED
