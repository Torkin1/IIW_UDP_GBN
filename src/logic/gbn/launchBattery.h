#ifndef LAUNCHBATTERY_H_INCLUDED
#define LAUNCHBATTERY_H_INCLUDED

#include <stdbool.h>
#include <pthread.h>
#include "gbn/packet.h"
#include "gbn/window.h"
#include "gbn/sortingTable.h"
#include <time.h>

// max num of packets that can occupy the queue. must be much greater than sendWindo
#define QUEUE_LEN 1024//100   

extern pthread_mutex_t sendWindowLock;
extern pthread_mutex_t sendTableLock;
extern pthread_mutex_t launchBatteryLock;

// Each launch pad can only be in one of the following statuses
typedef enum launchPadStatus{

    EMPTY,                      // launch pad has no packet in it
    READY,                      // hosted packet is ready to be sent the first time
    SENT,                       // hosted packet has already been sent at least one time and it is waiting for ACK
    ACKED,                      // hosted packet is acked
    LOST,                       // hosted packet has no chance to be sent (e.g max rtx attempts reached)
    NUM_LAUNCHPAD_STATUSES

} LaunchPadStatus;

// Every seat in the launch battery is a launch pad, which can hold a packet and tracks its status
typedef struct launchPad{

    Packet *packet;                 // packet in the seat
    unsigned int launches;          // number of times the packet in the seat has been transmitted
    LaunchPadStatus status;         // status of launch pad
    struct timespec launchTime;     // abs time of last launch


} LaunchPad;

// Send queue can be imagined as a launch battery, where each seat is a launch pad.
typedef struct launchBattery{

    LaunchPad *battery[QUEUE_LEN];          // the actual queue

                                            /*
                                                tracks number of contiguous pads available.
                                                This redundancy is useful because it kills the need to check every pad to know if there is enough space to add more packets.
                                                Must be updated whenever new packets are added or packets in battery are ACKED or LOST
                                            */
    int contiguousPadsAvailable;
    int nextAvailableIndex;                 // index next to the last pad occupied

} LaunchBattery;

// @return reference to singleton launch battery reference
LaunchBattery *getLaunchBatteryReference();

/*
    Destroys every pad in the battery, then frees memory pointed from self
    @param self poiner to battery target
*/
int destroyLaunchBattery(LaunchBattery *self);

// tells if a launch pad is available to host a new packet
bool isLaunchPadEmpty(LaunchPad *self);

// adds packets to launchbattery
int addToLaunchBattery(Packet *packets[], int n);

// tells if there is enough space in the battery for n packets
bool willTheyFit(int n);

// call this to keep track of contiguous pads in battery
int updateContiguousPads(int change);

// stores the send window reference
SendWindow *getSendWindowReference();

// true if i is in send window
bool isInWindow(int i);

// stores send table reference
SortingTable *getSendTableReference();

#endif // LAUNCHBATTERY_H_INCLUDED
