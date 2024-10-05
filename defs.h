#ifndef DEFS_H_
#define DEFS_H_

// Includes
#include <pthread.h>
#include <queue>
#include <iostream>
#include <vector>
#include "const.h"

// Classes & Typedefs
class Packet{
public:
    int id;
    int priority;
    float arrivalTime;
    float startProcessingTime = 0; // when switching fabric processes this packet
    float sentTime = 0;        // when finally sent on output link
    // startProcessingTime is 0 for packets dropped on input queue
    // sentTime is 0 for packets dropped on output queue
    // wait whats the sense of output queue packet dropping?
    int inputPort;
    int outputPort;      // Forwarding Table, what's that?

    Packet(int id, int priority, float arrivalTime, int inputPort, int outputPort){
        this->id = id;
        this->priority = priority;
        this->arrivalTime = arrivalTime;
        this->inputPort = inputPort;
        this->outputPort = outputPort;
    }
    
    Packet* clone(){
        return new Packet(this->id, this->priority, this->arrivalTime, this->inputPort, this->outputPort);
    }
};

class Buffer{
    int capacity = BUFFER_CAPACITY;
    int size = 0;
    std::queue<Packet*> bufferQueue;

public:
    int push(Packet*);
    void pop();
    Packet* front();
    inline bool full() const { return size == capacity; }
    inline bool empty() const { return size == 0; }
    inline int getSize() const { return size; } 
};

class Router{
public:
    Buffer input[NUM_QUEUES];
    Buffer output[NUM_QUEUES];

    // Link layer functions
    int addToInputQueue(int, Packet*);
    void removeFromOutputQueue(int);

    // Scheduler functions
    Packet* removeFromInputQueue(int);
    int sendToOutputQueue(int, Packet*);
};

// Utility Functions
void trackSize(Router*);
void readAllPackets(std::vector<Packet>*);
std::ostream &operator<<(std::ostream &os, Packet const &pkt);
void printTransmitted(std::vector<Packet>*);
int getTime();

// The below 2 functions simulate the link layer
void sendToQueue(Router*, int);
void removeFromQueue(Router*, int);

// Scheduler Function
void PriorityScheduler(Router*);
void RoundRobinScheduler(Router*);
void WeightedFairScheduler(Router*);

#endif //DEFS_H_