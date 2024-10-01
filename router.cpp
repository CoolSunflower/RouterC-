#include <queue>
#define NUM_QUEUES 8

using namespace std;

class Packet{
    int id;
    int priority;
    int arrivalTime;
    // sentTime is 0 for dropped packets
    int sentTime = 0;
};

int time = 0; // Global variable managed by a seperate thread
typedef queue<Packet> inputQueue;
typedef queue<Packet> outputQueue;

class Router{
    inputQueue input[NUM_QUEUES];
    outputQueue output[NUM_QUEUES];

};

int main(int argc, char* argv[]){
    // Start seperate thread for managing time


    // Instantiate Router
    Router *router = new Router;

    // Start thread for inputting to queue
    // As of now all queues get equal data at equal time rate
    


    return 0;
}