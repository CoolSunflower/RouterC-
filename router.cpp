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

    Development Phases and TODOs:

    P1: Round Robin Scheduling without Metrics DONE
        Current Flow:
            Packets sent async by sendToQueue 
            Packets retreived from inputQueue and sent to correct outputQueue by scheduler
            Packets removed async by removeFromQueue
            All transmitted packets stored in an array and printed at end

    P2: Add support for Buffers, Metrics, Flow Types DONE

    P3: Priority and Weighted Fair Scheduling Algorithms

    P4: iSlip Algorithm

    TODO:
        - Input Queues with locks: DONE
            - Send to input queue with sleep for each router input port DONE
        - Output Queues with locks: DONE
            - Clear from each output port at fixed rate DONE
        - Separate threads layout for sending packets DONE
        - Separate threads for clearing output queues at a fixed rate DONE
        - Simple round-robin scheduler DONE

        - Queues --> Buffers Everywhere DONE
        - Increase Time precision DONE
        - Read Saved Data for metric calculation DONE
        - Change Time to System Time DONE
        - Metric Support DONE
        - Input Flow Types DONE
        - Dont read all packets into a vector (Segmentation Fault for large simulation time) DONE

        - Calculate Metrics in another file DONE
        - Priority Scheduler DONE


        - ...
        - do I really need a pid?


        Adding Support for metrics:
            1. Queue Throughput: The number of packets processed per unit time.
                - there will be an overall throughput and a per queue throughput
            2. Turnaround Time: The total time taken from when a packet or job enters the system (i.e., arrives
            in the queue) until it is completely processed and exits the system.
            3. Waiting Time: The total time a packet spends waiting in the queue before being serviced (i.e.,
            before it begins processing).
            4. Buffer Occupancy: Track the buffer occupancy for input and output queues.
            5. Packet Drop Rate: The percentage of packets that are dropped due to queue overflow
*/

// Includes
// #include <cstddef>
// #include <memory>
#include <thread>
#include <map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include "defs.h"
using namespace std;

// Global Variables
mutex inputMutex[8];     // Input queue mutexes <-- will be used when scheduler implemented 
mutex outputMutex[8];    // Output queue mutexes <-- will be used when scheduler implemented
std::atomic<bool> stop_threads(false);
int pid = 0;             // packet id's
mutex pidMutex;
// vector<Packet> transmitted; // All packets successfully transmitted
// Actually we want to maintain all packets!
map<int, Packet*> allPackets;
std::chrono::system_clock::time_point startTime;

int main(int argc, char* argv[]){
    startTime = std::chrono::system_clock::now();

    // Instantiate Router
    Router *router = new Router;

    // Create & Detach appropriate Scheduler thread
    int scheduler_choice;
    cout << "Select Scheduler:\n1. Priority Scheduling\n2. Weighted Fair Queuing\n3. Round Robin\n4. iSLIP\n";
    cin >> scheduler_choice;
    if(scheduler_choice == 1){
        thread scheduler(PriorityScheduler, router);
        scheduler.detach();
    }else if(scheduler_choice == 2){
        thread scheduler(WeightedFairScheduler, router);
        scheduler.detach();
    }else if(scheduler_choice == 3){
        thread scheduler(RoundRobinScheduler, router);
        scheduler.detach();
    }else if(scheduler_choice == 4){
        cout << "wait\n";
    }else{
        cout << "Invalid Choice. Exiting...\n";
        return 1;
    }

    freopen("output.txt", "w", stdout);

    // Create & Detach Thread for Tracking Size of each queue
    // needed to calculate buffer occupancy
    thread SizeThread(trackSize, router);
    SizeThread.detach();

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

    // Sleep for some time and then kill all threads
    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME));
    stop_threads.store(true);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    freopen("/dev/tty", "w", stdout);  // For Linux/Mac
    cout << "DONE SIMULATION!\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}

void PriorityScheduler(Router* router){
    // Queues 0, 1, 4, 5 have high priority traffic
    // Queues 2, 3, 6, 7 have low priority traffic
    while(!stop_threads.load()){
        // Based on priority
        // we need to decide which queue to take data from
        int selectedQueue;
        if(!(router->input[0].empty()))
            selectedQueue = 0;
        else if(!(router->input[4].empty()))
            selectedQueue = 4;
        else if(!(router->input[1].empty()))
            selectedQueue = 1;
        else if(!(router->input[5].empty()))
            selectedQueue = 5;
        else if(!(router->input[2].empty()))
            selectedQueue = 2;
        else if(!(router->input[6].empty()))
            selectedQueue = 6;
        else if(!(router->input[3].empty()))
            selectedQueue = 3;
        else if(!(router->input[7].empty()))
            selectedQueue = 7;
        else 
            continue;

        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(selectedQueue);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(10));

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);
    }
}

void RoundRobinScheduler(Router *router){
    // all it does is iterates thruogh the input queues, takes one and sends it to the output queues with waits in between
    int currQueue = 0;
    // this_thread::sleep_for(chrono::seconds(1));
    while(!stop_threads.load()){
        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(currQueue);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(10));

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);

        // Go to next Queue
        currQueue = (currQueue+1)%8;
    }
}

void sendToQueue(Router * router, int inputQueueNumber){
    while(!stop_threads.load()){
        // The sleep pattern and priority will depend on the queue number
        if((inputQueueNumber == 0) || (inputQueueNumber == 4)){ // Bursty High Priority Data
            // Sample Number of packets to be sent in burst
            int numPackets = BURST_LOW + rand()%(BURST_HIGH - BURST_LOW + 1);
            int priority = 0; // (lower is higher)

            // Allocate that many PIDs
            pidMutex.lock();
            int basePID = pid + 1;
            int maxPID = pid + numPackets;
            pid += numPackets;
            pidMutex.unlock();

            // Create & Send numPackets packets
            for(int i = basePID; i <= maxPID; i++){
                Packet* currPacket = new Packet(i, priority, getTime(), inputQueueNumber, rand()%8);
                allPackets.emplace(i, currPacket);

                // Push to Router
                router->addToInputQueue(inputQueueNumber, currPacket);

                // Sleep for 5-10 ms before sending another packet to this queue
                this_thread::sleep_for(chrono::milliseconds(5 + ( std::rand() % ( 10 - 5 + 1 ) ))); 
            }
            
            // Sleep for 2-5 seconds between bursts
            this_thread::sleep_for(chrono::milliseconds(2000 + ( std::rand() % ( 5000 - 2000 + 1 ) ))); 
        }else if((inputQueueNumber == 1) || (inputQueueNumber == 5)){ // Uniform High Priority Traffic
            pidMutex.lock();
            int currPID = ++pid;
            pidMutex.unlock();
            int priority = 0;

            Packet* currPacket = new Packet(currPID, priority, getTime(), inputQueueNumber, rand()%8);
            allPackets.emplace(currPID, currPacket);

            router->addToInputQueue(inputQueueNumber, currPacket);

            // Sleep for 40-80 ms to simulate constant traffic
            this_thread::sleep_for(chrono::milliseconds(40 + rand()%(80 - 40 + 1)));
        }else if((inputQueueNumber == 2) || (inputQueueNumber == 6)){ // Bursty Low Priority Traffic
            // Sample Number of packets to be sent in burst
            int numPackets = BURST_LOW + rand()%(BURST_HIGH - BURST_LOW + 1);
            int priority = 1; 

            // Allocate that many PIDs
            pidMutex.lock();
            int basePID = pid + 1;
            int maxPID = pid + numPackets;
            pid += numPackets;
            pidMutex.unlock();

            // Create & Send numPackets packets
            for(int i = basePID; i <= maxPID; i++){
                Packet* currPacket = new Packet(i, priority, getTime(), inputQueueNumber, rand()%8);
                allPackets.emplace(i, currPacket);

                // Push to Router
                router->addToInputQueue(inputQueueNumber, currPacket);

                // Sleep for 5-10 ms before sending another packet to this queue
                this_thread::sleep_for(chrono::milliseconds(5 + ( std::rand() % ( 10 - 5 + 1 ) ))); 
            }
            
            // Sleep for 2-5 seconds between bursts
            this_thread::sleep_for(chrono::milliseconds(2000 + ( std::rand() % ( 5000 - 2000 + 1 ) )));             
        }else if((inputQueueNumber == 3) || (inputQueueNumber == 7)){ // Uniform Low Priority Traffic
            pidMutex.lock();
            int currPID = ++pid;
            pidMutex.unlock();
            int priority = 1;

            Packet* currPacket = new Packet(currPID, priority, getTime(), inputQueueNumber, rand()%8);
            allPackets.emplace(currPID, currPacket);

            router->addToInputQueue(inputQueueNumber, currPacket);

            // Sleep for 40-80 ms to simulate constant traffic
            this_thread::sleep_for(chrono::milliseconds(40 + rand()%(80 - 40 + 1)));
        }
    }
}

void removeFromQueue(Router * router, int outputQueueNumber){
    while(!stop_threads.load()){
        router->removeFromOutputQueue(outputQueueNumber);
        this_thread::sleep_for(chrono::milliseconds(75));
    }
}

int Buffer::push(Packet* pkt){
    if(this->full()){ return 0;} // Signified packet dropping
    this->bufferQueue.push(pkt);
    this->size += 1;
    return pkt->id;
}

void Buffer::pop(){
    this->bufferQueue.pop();
    this->size -= 1;
}

Packet* Buffer::front(){
    return this->bufferQueue.front();
}

int Router::addToInputQueue(int inputQueueNumber, Packet* pkt){
    inputMutex[inputQueueNumber].lock();
    int ret = this->input[inputQueueNumber].push(pkt);
    // this->input[inputQueueNumber].push(*pkt);
    inputMutex[inputQueueNumber].unlock();
    if(ret == 0){
        // if packet was dropped, we need to print its information for tracking
        // since it was never added to input queue, this info will never be printed
        cout << *pkt;
        delete pkt;
    }
    return ret;
}

int Router::sendToOutputQueue(int outputQueueNumber, Packet* pkt){
    outputMutex[outputQueueNumber].lock();
    int ret = this->output[outputQueueNumber].push(pkt);
    // this->output[outputQueueNumber].push(*pkt);
    outputMutex[outputQueueNumber].unlock();
    return ret;
}

void Router::removeFromOutputQueue(int outputQueueNumber){
    while(this->output[outputQueueNumber].empty());

    outputMutex[outputQueueNumber].lock();
    Packet* pkt = this->output[outputQueueNumber].front();
    this->output[outputQueueNumber].pop();
    map<int, Packet*>::iterator it = allPackets.find(pkt->id);
    if(it != allPackets.end()){

        it->second->sentTime = getTime();

        // to avoid unnecessary storage, we can store the information of the packet in a file and delete it
        cout << *it->second;
        Packet* pktptr = it->second;
        allPackets.erase(it);
        delete pktptr;
    }

    outputMutex[outputQueueNumber].unlock();
}

Packet* Router::removeFromInputQueue(int inputQueueNumber){
    while(this->input[inputQueueNumber].empty());

    inputMutex[inputQueueNumber].lock();
    Packet* pkt = this->input[inputQueueNumber].front();
    this->input[inputQueueNumber].pop();
    if(allPackets.find(pkt->id) != allPackets.end()){
        allPackets.find(pkt->id)->second->startProcessingTime = getTime();
    }
    inputMutex[inputQueueNumber].unlock();

    return pkt;
}

ostream &operator<<(ostream &os, Packet const &pkt) { 
    return os << pkt.id << " " << pkt.priority << " " << pkt.inputPort << " " << pkt.outputPort << " " << pkt.arrivalTime << " " << pkt.startProcessingTime << " " << pkt.sentTime << "\n";
}

void trackSize(Router* router){
    std::vector<std::vector<int>> inputSizes;  
    std::vector<std::vector<int>> outputSizes; 

    std::ofstream outFile("queue_sizes.txt"); // Output file stream
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    while (!stop_threads.load()) {
        std::vector<int> currentInputSizes;
        std::vector<int> currentOutputSizes;

        for (int i = 0; i < NUM_QUEUES; i++) {
            currentInputSizes.push_back(router->input[i].getSize());
        }

        for (int i = 0; i < NUM_QUEUES; i++) {
            currentOutputSizes.push_back(router->output[i].getSize());
        }

        inputSizes.push_back(currentInputSizes);
        outputSizes.push_back(currentOutputSizes);

        outFile << "Time: " << getTime() << "\n";
        outFile << "Input Queue Sizes: ";
        for (const auto& size : currentInputSizes) {
            outFile << size << " ";
        }
        outFile << "\n";

        outFile << "Output Queue Sizes: ";
        for (const auto& size : currentOutputSizes) {
            outFile << size << " ";
        }
        outFile << "\n\n";
        outFile.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    outFile.close();
}

int getTime(){
    // std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    // auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    // return timeSinceEpoch - ;
    std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
    auto elapsedTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    return elapsedTimeMillis;
}

void printTransmitted(vector<Packet>* packets){
    for(auto pkt: *packets)
        cout << pkt;
}

