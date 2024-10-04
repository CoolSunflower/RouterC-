#ifndef DEFS_H_
#define DEFS_H_

// Includes
#include <pthread.h>
#include <queue>

// Defines
#define NUM_QUEUES 8
#define SIMULATION_TIME 15
#define BUFFER_CAPACITY 64

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

class Buffer{
    int capacity = BUFFER_CAPACITY;
    int size = 0;
    std::queue<Packet> bufferQueue;

public:
    int push(Packet);
    void pop();
    Packet front();
    inline bool full() const {
        return size == capacity;
    }
    inline bool empty() const {
        return size == 0;
    }
};

class Router{
private:
    Buffer input[NUM_QUEUES];
    Buffer output[NUM_QUEUES];

    // std::queue<Packet> input[NUM_QUEUES];
    // std::queue<Packet> output[NUM_QUEUES];

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
void printTransmitted();

// The below 2 functions simulate the link layer
void sendToQueue(Router*, int);
void removeFromQueue(Router*, int);

// Scheduler Function
// void PriorityScheduler(Router*);
void RoundRobinScheduler(Router*);

#endif //DEFS_H_