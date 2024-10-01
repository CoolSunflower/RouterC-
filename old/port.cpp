#include "port.h"

Port::Port() {
    buffer_occupancy = 0;
    packets_dropped = 0;
}

bool Port::enqueue(const Packet& pkt) {
    if (buffer.size() < BUFFER_SIZE) {
        buffer.push(pkt);
        buffer_occupancy++;
        return true;
    } else {
        packets_dropped++;
        return false;
    }
}

Packet Port::dequeue() {
    Packet pkt = buffer.front();
    buffer.pop();
    buffer_occupancy--;
    return pkt;
}

bool Port::is_empty() {
    return buffer.empty();
}

int Port::size() {
    return buffer.size();
}
