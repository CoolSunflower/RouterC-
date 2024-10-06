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

    P3: Priority and Weighted Fair Scheduling Algorithms DONE

    P4: iSlip Algorithm DONE

    TODOs:
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
        - Weighted Fair Scheduler DONE

        - iSLIP Scheduler: DONE
            - major scheduler code DONE
            - add VOQ DONE
*/

// Includes
#include <thread>
#include <map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <sys/stat.h> 
#include <sys/wait.h> 
#ifdef _WIN32
    #include <direct.h> // For _mkdir() on Windows
#else
    #include <unistd.h> // For access() on Linux
#endif
#include "const.h"
#include "defs.h"
using namespace std;

// Global Variables
mutex inputMutex[NUM_QUEUES];                   // Input queue mutexes <-- will be used when scheduler implemented 
mutex inputMutexVOQ[NUM_QUEUES][NUM_QUEUES];    // used for VOQ required in iSLIP algorithm
mutex outputMutex[NUM_QUEUES];                  // Output queue mutexes <-- will be used when scheduler implemented
std::atomic<bool> stop_threads(false);       // For stopping all threads after simulationt time is finished
int pid = 0;                                    // packet id's
mutex pidMutex;                                 // mutex over PID
map<int, Packet*> allPackets;                   // map to maintain all dynamically allocated packets
std::chrono::system_clock::time_point startTime;// startTime is used by getTime() for populating time values in packets
int scheduler_choice;                           // global scheduler choice used to take scheduler specific actions, if any
mutex consoleMutex;                             // console mutex ig understandable
string queueFileName;                           // File name to store buffer tracking data
int typeOfSimulation = 1;                       // Uniform, Non-Uniform or Bursty
/*
    0 = Uniform Simulation --> All Input Queues receive same priority uniform rate data
    1 = Non-Uniform Simulation --> Different Priorities Uniform Traffic
    2 = Bursty Simulation --> Different Queues recieve different types (uniform v/s bursty) and different priority data
*/

int main(int argc, char* argv[]){
    startTime = std::chrono::system_clock::now(); // Set the startTime variable to the current time

    Router *router = new Router; // Instantiate Router

    // Create & Detach appropriate Scheduler thread based on user input
    cout << "Select Scheduler:\n1. Priority Scheduler\n2. Weighted Fair Scheduler\n3. Round Robin Scheduler\n4. iSLIP Scheduler\n";
    cin >> scheduler_choice;
    string filename;
    if(scheduler_choice == 1){
        thread scheduler(PriorityScheduler, router);
        scheduler.detach();
        filename = "./SimulationOutput/PriorityScheduler.txt";
        queueFileName = "./SimulationOutput/PrioritySchedulerQueue.txt";
    }else if(scheduler_choice == 2){
        thread scheduler(WeightedFairScheduler, router);
        scheduler.detach();
        filename = "./SimulationOutput/WeightedFairScheduler.txt";
        queueFileName = "./SimulationOutput/WeightedFairSchedulerQueue.txt";
    }else if(scheduler_choice == 3){
        thread scheduler(RoundRobinScheduler, router);
        scheduler.detach();
        filename = "./SimulationOutput/RoundRobinScheduler.txt";
        queueFileName = "./SimulationOutput/RoundRobinSchedulerQueue.txt";
    }else if(scheduler_choice == 4){
        thread scheduler(iSLIPScheduler, router);
        scheduler.detach();
        filename = "./SimulationOutput/iSLIPScheduler.txt";
        queueFileName = "./SimulationOutput/iSLIPSchedulerQueue.txt";
    }else{
        cout << "Invalid Choice. Exiting...\n";
        return 1;
    }

    // Change output file to store packet data
    createFoler("SimulationOutput/");
    createFoler("SimulationReports/");
    freopen(filename.c_str(), "w", stdout);

    // Create & Detach Thread for Tracking Size of each queue
    thread SizeThread(trackSize, router);
    SizeThread.detach();

    // START Link Layer Functions:
    // Start detached threads for inputting to queues
    vector<thread> InputDataSenderThreads;
    for(int i = 0; i < NUM_QUEUES; i++)
        InputDataSenderThreads.push_back(thread(sendToQueue, router, i));
    for(int i = 0; i < NUM_QUEUES; i++)
        InputDataSenderThreads[i].detach();
    // Start detached threads for removing data from output queues
    vector<thread> OutputQueueRemover;
    for(int i = 0; i < NUM_QUEUES; i++)
        OutputQueueRemover.push_back(thread(removeFromQueue, router, i));
    for(int i = 0; i < NUM_QUEUES; i++)
        OutputQueueRemover[i].detach();

    // Sleep for some time and then kill all threads
    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME));
    stop_threads.store(true);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    #ifdef _WIN32
        freopen("CON", "w", stdout);  // For Windows
    #else
        freopen("/dev/tty", "w", stdout); // For Linux/Mac
    #endif

    cout << "DONE SIMULATION!\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Get Metrics
    int pid = fork();
    if(pid == 0){
        // Child process: execute ./metrics
        execl("./metrics", "./metrics", to_string(scheduler_choice).c_str(), (char *) nullptr);
        std::cerr << "Error executing ./metrics" << std::endl;
        return 1;
    }else{
        // Parent process: wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);  // Wait for the child
    }

    return 0;
}

void PriorityScheduler(Router* router){
    // Queues 0, 1, 4, 5 have high priority traffic
    // Queues 2, 3, 6, 7 have low priority traffic
    while(!stop_threads.load()){
        // Based on priority
        // we need to decide which queue to take data from
        int selectedQueue;
        if(typeOfSimulation == 0){
            // Uniform Simulation (all queues have same priority traffic)
            selectedQueue = rand()%8;
        }else if(typeOfSimulation == 1 || typeOfSimulation == 2){
            // Non Uniform Simulation (some queues have higher priority)
            // high -- 0, 4, 5, 2    low -- 2, 6, 3, 7
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
        }

        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(selectedQueue);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(SWITCH_DELAY));

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);
    }
}

void WeightedFairScheduler(Router* router){
    // WFQ Scheduler can be implemented probablistically using Lottery Scheduling
    // Setting tickets 
    int tickets[NUM_QUEUES] = {};
    if(typeOfSimulation == 0){
        tickets[0] = tickets[4] = 70;         
        tickets[1] = tickets[5] = 70; 
        tickets[2] = tickets[6] = 70;        
        tickets[3] = tickets[7] = 70;     
    }else if(typeOfSimulation == 1 || typeOfSimulation == 2){
        tickets[0] = tickets[4] = 100; // High Priority Bursty Traffic
        tickets[1] = tickets[5] = 90; // High Priority Uniform Traffic
        tickets[2] = tickets[6] = 50; // Low Priority Bursty Traffic
        tickets[3] = tickets[7] = 40; // Low Priority Uniform Traffic
    }
    int totalTickets = 560;
    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 gen(rd()); // Seed the Mersenne Twister random number generator
    std::uniform_int_distribution<> distrib(1, totalTickets);

    while(!(stop_threads.load())){
        // Sample queue
        int winningTickets = distrib(gen);
        int currentTickets = 0;
        int selectedQueue = 0;

        for(; selectedQueue < NUM_QUEUES; selectedQueue++){
            currentTickets += tickets[selectedQueue];
            if (winningTickets <= currentTickets) {
                break;
            }
        }

        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(selectedQueue);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(SWITCH_DELAY));

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);
    }
}

void RoundRobinScheduler(Router* router){
    // all it does is iterates thruogh the input queues, takes one and sends it to the output queues with waits in between
    int currQueue = 0;
    // this_thread::sleep_for(chrono::seconds(1));
    while(!stop_threads.load()){
        // Get packet from Router Current Input Queue
        Packet* pkt = router->removeFromInputQueue(currQueue);

        // Sleep to simulate switching fabric delay
        this_thread::sleep_for(chrono::milliseconds(SWITCH_DELAY));

        // Send to Corrent Output Queue
        router->sendToOutputQueue(pkt->outputPort, pkt);

        // Go to next Queue
        currQueue = (currQueue+1)%8;
    }
}

void iSLIPScheduler(Router* router){
    // Round-robin pointers for inputs and outputs
    std::vector<int> inputPointers(NUM_QUEUES, 0);
    std::vector<int> outputPointers(NUM_QUEUES, 0);

    while (!stop_threads.load()) {
        std::vector<int> grantedOutputs(NUM_QUEUES, -1); // Track which output has granted to which input
        std::vector<int> acceptedInputs(NUM_QUEUES, -1); // Track which input has accepted which output

        // Step 1: Request Phase
        // Each input requests to all outputs for which it has packets queued
        std::vector<std::vector<bool>> requests(NUM_QUEUES, std::vector<bool>(NUM_QUEUES, false));
        for (int i = 0; i < NUM_QUEUES; i++) {
            for (int j = 0; j < NUM_QUEUES; j++) {
                if(!router->VOQInput[i].empty(j)){
                    requests[i][j] = true;
                }
            }
        }

        // Step 2: Grant Phase
        // Each output reviews the requests and grants to one input
        for (int j = 0; j < NUM_QUEUES; j++) {
            for (int i = 0; i < NUM_QUEUES; i++) {
                int inputIndex = (outputPointers[j] + i) % NUM_QUEUES;
                if (requests[inputIndex][j]) {
                    grantedOutputs[j] = inputIndex;  // Output j grants to inputIndex
                    break;
                }
            }
        }

        // Step 3: Accept Phase
        // Each input that received one or more grants selects one output
        for (int i = 0; i < NUM_QUEUES; i++) {
            // Round Robin for input pointers
            for (int j = 0; j < NUM_QUEUES; j++) {
                int outputIndex = (inputPointers[i] + j) % NUM_QUEUES;
                if (grantedOutputs[outputIndex] == i) {
                    acceptedInputs[i] = outputIndex;  // Input i accepts output outputIndex
                    break;
                }else{
                    acceptedInputs[i] = -1;
                }
            }
        }

        // Packet Transfer based on accepted grants
        for (int i = 0; i < NUM_QUEUES; i++) {
            if (acceptedInputs[i] != -1) {
                int selectedOutput = acceptedInputs[i];
                
                // Remove packet from input queue and send to output queue
                Packet* pkt = router->removeFromInputQueueVOQ(i, selectedOutput);

                if (pkt) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(SWITCH_DELAY));  // Simulate switching delay
                    router->sendToOutputQueue(selectedOutput, pkt);

                    // Update the round-robin pointer only if grant was accepted
                    inputPointers[i] = (inputPointers[i] + 1) % NUM_QUEUES;
                    outputPointers[selectedOutput] = (outputPointers[selectedOutput] + 1) % NUM_QUEUES;
                }

                requests[i][selectedOutput] = false;
            }
        }

        // Small sleep to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }}

// Link Layer Adding Function
void sendToQueue(Router * router, int inputQueueNumber){
    int priority[NUM_QUEUES];
    int type[NUM_QUEUES]; // 0 - Bursty, 1 - Uniform
    if(typeOfSimulation == 0){
        for(int i = 0; i < NUM_QUEUES; i++){
            priority[i] = 0; // All High Priority
            type[i] = 1; // Uniform Traffic
        }
    }else if(typeOfSimulation == 1){
        for(int i = 0; i < NUM_QUEUES; i++){
            type[i] = 1; // All Uniform Traffic
        }
        priority[0] = priority[4] = priority[2] = priority[6] = 0; // High Priority Traffic
        priority[1] = priority[5] = priority[3] = priority[7] = 1; // Low Priority Traffic
    }else if(typeOfSimulation == 2){
        type[0] = type[4] = type[2] = type[6] = 0; // Bursty Traffic
        type[1] = type[5] = type[3] = type[7] = 1; // Uniform Traffic
        priority[0] = priority[4] = priority[2] = priority[6] = 0; // High Priority Traffic
        priority[1] = priority[5] = priority[3] = priority[7] = 1; // Low Priority Traffic
    }

    while(!stop_threads.load()){
        // The sleep pattern and priority will depend on the queue number
        if(type[inputQueueNumber] == 0){ // Bursty Traffic
            // Sample Number of packets to be sent in burst
            int numPackets = BURST_LOW + rand()%(BURST_HIGH - BURST_LOW + 1);

            // Allocate that many PIDs
            pidMutex.lock();
            int basePID = pid + 1;
            int maxPID = pid + numPackets;
            pid += numPackets;
            pidMutex.unlock();

            // Create & Send numPackets packets
            for(int i = basePID; i <= maxPID; i++){
                Packet* currPacket = new Packet(i, priority[inputQueueNumber], getTime(), inputQueueNumber, rand()%8);
                allPackets.emplace(i, currPacket);

                // Push to Router
                router->addToInputQueue(inputQueueNumber, currPacket);

                // Sleep for 3-5 ms before sending another packet to this queue
                this_thread::sleep_for(chrono::milliseconds(3 + ( std::rand() % ( 5 - 3 + 1 ) ))); 
            }
            
            // Sleep for 2.5-4 seconds between bursts
            this_thread::sleep_for(chrono::milliseconds(2500 + ( std::rand() % ( 4000 - 2500 + 1 ) ))); 
        }else if(type[inputQueueNumber] == 1){ // Uniform Traffic
            pidMutex.lock();
            int currPID = ++pid;
            pidMutex.unlock();

            Packet* currPacket = new Packet(currPID, priority[inputQueueNumber], getTime(), inputQueueNumber, rand()%8);
            allPackets.emplace(currPID, currPacket);

            router->addToInputQueue(inputQueueNumber, currPacket);

            // Sleep for 40-80 ms to simulate constant traffic
            this_thread::sleep_for(chrono::milliseconds(40 + rand()%(80 - 40 + 1)));
        }
    }
}

// Link Layer Removing Function
void removeFromQueue(Router * router, int outputQueueNumber){
    while(!stop_threads.load()){
        router->removeFromOutputQueue(outputQueueNumber);
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

// Buffer push functions
int Buffer::push(Packet* pkt){
    if(this->full()){ return 0;} // Signified packet dropping
    this->bufferQueue.push(pkt);
    this->size += 1;
    return pkt->id;
}

// Buffer pop function
void Buffer::pop(){
    this->bufferQueue.pop();
    this->size -= 1;
}

// Buffer front function
Packet* Buffer::front(){
    return this->bufferQueue.front();
}

// Virtual Output Queueing (VOQ) Buffer push function
int VOQBuffer::push(Packet* pkt, int outputIndex){
    if(this->full()){ return 0; } // Packet dropping for this input port
    this->bufferQueue[outputIndex].push(pkt);
    this->sizeMutex.lock();
    this->size++;
    this->sizeMutex.unlock();
    return pkt->id;
}

// Virtual Output Queueing (VOQ) Buffer pop function
void VOQBuffer::pop(int outputIndex){
    this->bufferQueue[outputIndex].pop();
    this->sizeMutex.lock();
    this->size--;
    this->sizeMutex.unlock();
}

// Virtual Output Queueing (VOQ) Buffer front function
Packet* VOQBuffer::front(int outputIndex){
    return this->bufferQueue[outputIndex].front();
}

// Router Function to add to input queue (used by link layer adder)
int Router::addToInputQueue(int inputQueueNumber, Packet* pkt){
    if(scheduler_choice == 4){
        // for iSLIP algorithm we need to use VOQ
        inputMutexVOQ[inputQueueNumber][pkt->outputPort].lock();
        int ret = this->VOQInput[inputQueueNumber].push(pkt, pkt->outputPort);
        inputMutexVOQ[inputQueueNumber][pkt->outputPort].unlock();
        if(ret == 0){
            consoleMutex.lock();
            cout << *pkt;
            consoleMutex.unlock();
            delete pkt;
        }
        return ret;
    }
    // for other algorithms normal procedure is followed
    inputMutex[inputQueueNumber].lock();
    int ret = this->input[inputQueueNumber].push(pkt);
    inputMutex[inputQueueNumber].unlock();
    if(ret == 0){
        // if packet was dropped, we need to print its information for tracking
        // since it was never added to input queue, this info will never be printed
        consoleMutex.lock();
        cout << *pkt;
        consoleMutex.unlock();
        delete pkt;
    }
    return ret;
}

// Router function to send to output queue (used by scheduler to send packet to correct output queue)
int Router::sendToOutputQueue(int outputQueueNumber, Packet* pkt){
    outputMutex[outputQueueNumber].lock();
    int ret = this->output[outputQueueNumber].push(pkt);
    // this->output[outputQueueNumber].push(*pkt);
    outputMutex[outputQueueNumber].unlock();
    return ret;
}

// Router function to remove from output queue (used by link layer remover function to cleanup output queues)
void Router::removeFromOutputQueue(int outputQueueNumber){
    while(this->output[outputQueueNumber].empty());

    outputMutex[outputQueueNumber].lock();
    Packet* pkt = this->output[outputQueueNumber].front();
    this->output[outputQueueNumber].pop();
    map<int, Packet*>::iterator it = allPackets.find(pkt->id);
    if(it != allPackets.end()){

        it->second->sentTime = getTime();

        // to avoid unnecessary storage, we can store the information of the packet in a file and delete it
        consoleMutex.lock();
        cout << *it->second;
        consoleMutex.unlock();
        Packet* pktptr = it->second;
        allPackets.erase(it);
        delete pktptr;
    }

    outputMutex[outputQueueNumber].unlock();
}

// Router function to remove from input queue (used by scheduler to remove from any selected input queue for processing)
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

// Router function to reomve from input queue, modified for iSLIP since VOQ is used
Packet* Router::removeFromInputQueueVOQ(int inputQueueNumber, int outputIndex){
    while(this->VOQInput[inputQueueNumber].empty(outputIndex));

    inputMutexVOQ[inputQueueNumber][outputIndex].lock();
    Packet* pkt = this->VOQInput[inputQueueNumber].front(outputIndex);
    this->VOQInput[inputQueueNumber].pop(outputIndex);
    if(allPackets.find(pkt->id) != allPackets.end()){
        allPackets.find(pkt->id)->second->startProcessingTime = getTime();
    }
    inputMutexVOQ[inputQueueNumber][outputIndex].unlock();

    return pkt;
}

// Utility function to define output of Packet Class Objects
ostream &operator<<(ostream &os, Packet const &pkt) { 
    return os << pkt.id << " " << pkt.priority << " " << pkt.inputPort << " " << pkt.outputPort << " " << pkt.arrivalTime << " " << pkt.startProcessingTime << " " << pkt.sentTime << " \n";
}

// trackSize runs on a seperate thread and tracks input and output buffer Occupancy
void trackSize(Router* router){
    std::vector<std::vector<int>> inputSizes;  
    std::vector<std::vector<int>> outputSizes; 

    std::ofstream outFile(queueFileName); // Output file stream
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    while (!stop_threads.load()) {
        std::vector<int> currentInputSizes;
        std::vector<int> currentOutputSizes;

        for (int i = 0; i < NUM_QUEUES; i++) {
            if(scheduler_choice == 4){
                currentInputSizes.push_back(router->VOQInput[i].getSize());
            }else{
                currentInputSizes.push_back(router->input[i].getSize());
            }
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

// This function returns the relative time with respect to the start time
int getTime(){
    std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
    auto elapsedTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    return elapsedTimeMillis;
}

// utility function to create the SimulationResults folder if it does not exist
void createFoler(std::string folderPath){
    struct stat info;
    
    // Check if the folder exists
    if (stat(folderPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
    #ifdef _WIN32
            _mkdir(folderPath.c_str()) == 0;
    #else
            mkdir(folderPath.c_str(), 0777);
    #endif
    }
}