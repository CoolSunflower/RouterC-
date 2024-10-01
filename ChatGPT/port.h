#ifndef PORT_H
#define PORT_H

#include <queue>
#include "packet.h"

#define BUFFER_SIZE 64

class Port {
public:
    std::queue<Packet> buffer;
    int buffer_occupancy;
    int packets_dropped;

    Port();

    bool enqueue(const Packet& pkt);
    Packet dequeue();
    bool is_empty();
    int size();
};

#endif // PORT_H
