#ifndef DEFS_H_
#define DEFS_H_

// Includes
#include <queue>

// Defines
#define NUM_QUEUES 8
#define SIMULATION_TIME 5

// Classes & Typedefs
class Packet{
public:
    int id;
    int priority;
    int arrivalTime;
    int sentTime = 0;    // sentTime is 0 for dropped packets
    int inputPort;
    int outputPort;      // Forwarding Table, what's that?

    Packet(int id, int priority, int arrivalTime, int inputPort, int outputPort){
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

typedef std::queue<Packet> inputQueue;
typedef std::queue<Packet> outputQueue;

class Router{
    inputQueue input[NUM_QUEUES];
    outputQueue output[NUM_QUEUES];

public:
    // Link layer functions
    int addToInputQueue(int, Packet*);
    void removeFromOutputQueue(int);

    // Scheduler functions
    Packet* removeFromInputQueue(int);
    int sendToOutputQueue(int, Packet*);
};

// Utility Functions
void updateTime();
// The below 2 functions simulate the link layer
void sendToQueue(Router*, int);
void removeFromQueue(Router*, int);

// Scheduler Function
void RoundRobinScheduler(Router*);

#endif //DEFS_H_