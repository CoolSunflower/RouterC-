#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "const.h"
using namespace std;

void calculateMetrics();
string filename;
string reportfilename;

int main(int argc, char* argv[]){
    if(argc != 2){
        return 1;
    }
    int scheduler_choice = atoi(argv[1]);
    if(scheduler_choice == 1){
        filename = "./SimulationOutput/PriorityScheduler.txt";
        reportfilename = "./SimulationReports/PriorityScheduler.txt";
    }else if(scheduler_choice == 2){
        filename = "./SimulationOutput/WeightedFairScheduler.txt";
        reportfilename = "./SimulationReports/WeightedFairScheduler.txt";
    }else if(scheduler_choice == 3){
        filename = "./SimulationOutput/RoundRobinScheduler.txt";
        reportfilename = "./SimulationReports/RoundRobinScheduler.txt";
    }else if(scheduler_choice == 4){
        filename = "./SimulationOutput/iSLIPScheduler.txt";
        reportfilename = "./SimulationReports/iSLIPScheduler.txt";
    }else{
        cout << "Invalid Choice. Exiting...\n";
        return 1;
    }

    calculateMetrics();
    return 0;
}

void calculateMetrics(){
    std::ifstream inputFile(filename);

    // Check if the file opened successfully
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        exit(1);
    }

    std::ofstream logfile(reportfilename, std::ios::app);  // Open file for appending logs
    if (!logfile) {
        std::cerr << "Error opening file!" << std::endl;
        exit(1);
    }

    std::string line;
    int totalPackets = 0;
    int totalSuccessfullyTransmitted = 0;
    int totalDropped = 0;
    int queuewiseTotalPackets[NUM_QUEUES] = {};
    int queuewiseTotalSuccessfulPackets[NUM_QUEUES] = {};
    int queuewiseTotalDroppedPackets[NUM_QUEUES] = {};
    float totalWaitingTime = 0;
    float queuewiseTotalWaitingTime[NUM_QUEUES] = {};
    float totalTurnaroundTime = 0;
    float queuewiseTotalTurnaroundTime[NUM_QUEUES] = {};
    
    // Read the file line by line
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);

        int id, priority, inputPort, outputPort, arrivalTime, startProcessingTime, sentTime;

        // Parse space-separated integers from the line
        if (iss >> id >> priority >> inputPort >> outputPort >> arrivalTime >> startProcessingTime >> sentTime) {
            // Print each entry for the current line
            // std::cout << "ID: " << id 
            //           << ", Priority: " << priority
            //           << ", InputPort: " << inputPort
            //           << ", OutputPort: " << outputPort
            //           << ", ArrivalTime: " << arrivalTime
            //           << ", StartProcessingTime: " << startProcessingTime
            //           << ", SentTime: " << sentTime << std::endl;

            // Getting statistics
            totalPackets++;
            queuewiseTotalPackets[inputPort]++;
            if(sentTime != 0){ // Not a dropped packet
                queuewiseTotalSuccessfulPackets[inputPort]++;
                totalSuccessfullyTransmitted++;

                totalTurnaroundTime += sentTime - arrivalTime;
                queuewiseTotalTurnaroundTime[inputPort] += sentTime - arrivalTime;

                totalWaitingTime += startProcessingTime - arrivalTime;
                queuewiseTotalWaitingTime[inputPort] += startProcessingTime - arrivalTime;
            }else{
                queuewiseTotalDroppedPackets[inputPort]++;
                totalDropped++;
            }
        } else {
            // std::cerr << "Error parsing line: " << line << std::endl;
            continue;
        }
    }    
    
    inputFile.close();

    cout << "\n----- Simulation Results -----\n";
    cout << "== General Statistics ==\n";
    cout << "Total Packets Generated: " << totalPackets << "\n";
    cout << "Total Packets Successfully Transmitted: " << totalSuccessfullyTransmitted << "\n";
    cout << "Total Packets Dropped: " << totalDropped << "\n";

    // Queue Throughput
    cout << "\n== Queue Throughput ==\n";
    cout << "Combined Router Throughput: " << totalSuccessfullyTransmitted/(float)SIMULATION_TIME << " Packets/Second \n";
    cout << "Queue Throughput for each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        cout << "Queue " << i << ": " << queuewiseTotalSuccessfulPackets[i]/(float)SIMULATION_TIME << " Packets/Second \n";
    } 

    // Turnaround Time
    cout << "\n== Turnaround Time ==\n";
    cout << "Average Turnaround Time: " << totalTurnaroundTime/totalSuccessfullyTransmitted << " ms \n";
    cout << "Turnaround Time for Each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        cout << "Queue " << i << ": " << queuewiseTotalTurnaroundTime[i]/queuewiseTotalSuccessfulPackets[i] << " ms \n";
    }

    // Waiting Time
    cout << "\n== Waiting Time ==\n";
    cout << "Average Waiting Time: " << totalWaitingTime/totalSuccessfullyTransmitted << " ms \n";
    cout << "Waiting Time for Each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        cout << "Queue " << i << ": " << queuewiseTotalWaitingTime[i]/queuewiseTotalSuccessfulPackets[i] << " ms \n";
    }

    // Buffer Occupancy
    cout << "\n== Buffer Occupancy ==\n";
    cout << "For continous input and output buffer occupancy data, see corresponding file in Simulation Output\n";


    // Packet Drop Rate
    cout << "\n== Packet Drop Rates ==\n";
    cout << "Percentage of Total Packets Dropped: " << (totalDropped/(float)totalPackets)*100 << "%\n";
    cout << "Percentage of Packets Dropped for each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        cout << "Queue " << i << ": " << (queuewiseTotalDroppedPackets[i]/(float)queuewiseTotalPackets[i])*100 << "%\n";
    } 


    // Saving information to the logfile
    logfile << "\n----- Simulation Results -----\n";
    logfile << "== General Statistics ==\n";
    logfile << "Total Packets Generated: " << totalPackets << "\n";
    logfile << "Total Packets Successfully Transmitted: " << totalSuccessfullyTransmitted << "\n";
    logfile << "Total Packets Dropped: " << totalDropped << "\n";

    // Queue Throughput
    logfile << "\n== Queue Throughput ==\n";
    logfile << "Combined Router Throughput: " << totalSuccessfullyTransmitted/(float)SIMULATION_TIME << " Packets/Second \n";
    logfile << "Queue Throughput for each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        logfile << "Queue " << i << ": " << queuewiseTotalSuccessfulPackets[i]/(float)SIMULATION_TIME << " Packets/Second \n";
    } 

    // Turnaround Time
    logfile << "\n== Turnaround Time ==\n";
    logfile << "Average Turnaround Time: " << totalTurnaroundTime/totalSuccessfullyTransmitted << " ms \n";
    logfile << "Turnaround Time for Each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        logfile << "Queue " << i << ": " << queuewiseTotalTurnaroundTime[i]/queuewiseTotalSuccessfulPackets[i] << " ms \n";
    }

    // Waiting Time
    logfile << "\n== Waiting Time ==\n";
    logfile << "Average Waiting Time: " << totalWaitingTime/totalSuccessfullyTransmitted << " ms \n";
    logfile << "Waiting Time for Each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        logfile << "Queue " << i << ": " << queuewiseTotalWaitingTime[i]/queuewiseTotalSuccessfulPackets[i] << " ms \n";
    }

    // Buffer Occupancy
    logfile << "\n== Buffer Occupancy ==\n";
    logfile << "For continous input and output buffer occupancy data, see corresponding file in Simulation Output\n";

    // Packet Drop Rate
    logfile << "\n== Packet Drop Rates ==\n";
    logfile << "Percentage of Total Packets Dropped: " << (totalDropped/(float)totalPackets)*100 << "%\n";
    logfile << "Percentage of Packets Dropped for each Input Queue: \n";
    for(int i = 0; i < NUM_QUEUES; i++){
        logfile << "Queue " << i << ": " << (queuewiseTotalDroppedPackets[i]/(float)queuewiseTotalPackets[i])*100 << "%\n";
    } 
}