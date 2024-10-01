#ifndef PACKET_H
#define PACKET_H

struct Packet {
    int id;
    int priority;       // 0: low, 1: high
    int arrival_time;
    int size;
    int source_port;
    int dest_port;
    int start_service_time;
    int finish_time;
};

#endif // PACKET_H
