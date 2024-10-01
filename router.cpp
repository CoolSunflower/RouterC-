// I am gonna try to do my implementation without chatgpt anyways, because learning and stuff ig

/*
              +---------------------+
      I[0]  --|                     |
      I[1]  --|                     |
      I[2]  --|                     |
      I[3]  --|    Scheduling       |
      I[4]  --|         Algorithm   |
      I[5]  --|                     |
      I[6]  --|                     |
      I[7]  --|                     |
              +---------------------+
                |  |  |  |  |  |  |  
                O[0]    ...      O[7]

    Only 2 priorities: High/Low
    Different Input Queues receive different types of traffic: [high priority/ low priority] [bursty/constant]
    TODO:
        - Input Queues with locks: DONE
            - Send to input queue with sleep for each router input port DONE
        - Output Queues with locks: DONE
            - Clear from each output port at fixed rate DONE
        - Separate threads layout for sending packets DONE
        - Separate threads for clearing output queues at a fixed rate DONE
        - Simple round-robin scheduler DONE
        - Add metrics support:
            - return value in sendToQueue to symbolize if packet dropped
        - Abstract Queues, behind a seperate class with a fixed capacity
        - Different types of traffic: can simulate using if checks and sleep times
        - ...
*/

// Includes
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include "defs.h"
using namespace std;

// Global Variables
int routerTime = 0;      // Global time variable managed by a seperate thread
mutex inputMutex[8];     // Input queue mutexes <-- will be used when scheduler implemented 
mutex outputMutex[8];    // Output queue mutexes <-- will be used when scheduler implemented
std::atomic<bool> stop_threads(false);
int pid = 0;             // packet id's
mutex pidMutex;

int main(int argc, char* argv[]){
    // Seperate thread for managing time
    thread TimeThread(updateTime);
    TimeThread.detach();

    // Instantiate Router
    Router *router = new Router;

    // Start detached thread for inputting to queue
    vector<thread> InputDataSenderThreads;
    for(int i = 0; i < NUM_QUEUES; i++)
        InputDataSenderThreads.push_back(thread(sendToQueue, router, i));
    for(int i = 0; i < NUM_QUEUES; i++)
        InputDataSenderThreads[i].detach();

    // Start detached thread for removing from queue
    vector<thread> OutputQueueRemover;
    for(int i = 0; i < NUM_QUEUES; i++)
        OutputQueueRemover.push_back(thread(removeFromQueue, router, i));
    for(int i = 0; i < NUM_QUEUES; i++)
        OutputQueueRemover[i].detach();

    // Detach Scheduler thread
    int scheduler_choice;
    cout << "Select Scheduler:\n1. Priority Scheduling\n2. Weighted Fair Queuing\n3. Round Robin\n4. iSLIP\n";
    cin >> scheduler_choice;
    if(scheduler_choice == 1){
        cout << "wait\n";
    }else if(scheduler_choice == 2){
        cout << "wait\n";
    }else if(scheduler_choice == 3){
        thread scheduler(RoundRobinScheduler, router);
        scheduler.detach();
    }else if(scheduler_choice == 4){
        cout << "wait\n";
    }else if(scheduler_choice == 5){
        cout << "wait\n";
    }

    // Sleep for some time and then kill all threads
    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME));
    stop_threads.store(true);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}

void updateTime(){
    while(!stop_threads.load()) {
        routerTime++;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void sendToQueue(Router * router, int inputQueueNumber){
    while(!stop_threads.load()){
        // Creating Packet
        pidMutex.lock();
        Packet pkt = Packet(pid++, rand()%2, routerTime, inputQueueNumber, rand()%8);
        pidMutex.unlock();

        // Push to Router
        router->addToInputQueue(inputQueueNumber, &pkt);
    
        // Sleep for 100-1000 ms before sending another packet to this queue
        this_thread::sleep_for(chrono::milliseconds(100 + ( std::rand() % ( 1000 - 100 + 1 ) ))); 
    }
}

void removeFromQueue(Router * router, int outputQueueNumber){
    while(!stop_threads.load()){
        router->removeFromOutputQueue(outputQueueNumber);
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}


void RoundRobinScheduler(Router *router){
    // all it does is iterates thruogh the input queues, takes one and sends it to the output queues with waits in between
    int currQueue = 0;
    while(!stop_threads.load()){
        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(currQueue);

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(50));

        // Go to next Queue
        currQueue = (currQueue+1)%8;
    }
}

int Router::addToInputQueue(int inputQueueNumber, Packet* pkt){
    inputMutex[inputQueueNumber].lock();
    this->input[inputQueueNumber].push(*pkt);
    inputMutex[inputQueueNumber].unlock();
    return 0;
}

void Router::removeFromOutputQueue(int outputQueueNumber){
    while(this->output[outputQueueNumber].empty());

    outputMutex[outputQueueNumber].lock();
    Packet pkt = this->output[outputQueueNumber].front();
    this->output[outputQueueNumber].pop();
    pkt.sentTime = routerTime;
    outputMutex[outputQueueNumber].unlock();
}

Packet* Router::removeFromInputQueue(int inputQueueNumber){
    while(this->input[inputQueueNumber].empty());

    inputMutex[inputQueueNumber].lock();
    Packet* pkt = this->input[inputQueueNumber].front().clone();
    this->input[inputQueueNumber].pop();
    inputMutex[inputQueueNumber].unlock();

    return pkt;
}

int Router::sendToOutputQueue(int outputQueueNumber, Packet* pkt){
    outputMutex[outputQueueNumber].lock();
    this->output[outputQueueNumber].push(*pkt);
    outputMutex[outputQueueNumber].unlock();
    return 0;
}