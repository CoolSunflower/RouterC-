#include "scheduler.h"
#include <algorithm>
#include <queue>

using namespace std;

// Priority Scheduler Implementation
void PriorityScheduler::schedule(vector<Port>& input_ports,
                                 vector<Port>& output_ports,
                                 int current_time,
                                 vector<Packet>& departed_packets) {
    // For each input port
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!input_ports[i].is_empty()) {
            // Find high-priority packets first
            queue<Packet> temp_queue;
            bool found = false;
            while (!input_ports[i].is_empty()) {
                Packet pkt = input_ports[i].dequeue();
                if (pkt.priority == 1 && !found) {
                    // Send packet to output port
                    pkt.start_service_time = current_time;
                    if (output_ports[pkt.dest_port].enqueue(pkt)) {
                        found = true;
                    } else {
                        // Output buffer full, packet dropped
                    }
                } else {
                    temp_queue.push(pkt);
                }
            }
            // Restore input buffer
            while (!temp_queue.empty()) {
                input_ports[i].enqueue(temp_queue.front());
                temp_queue.pop();
            }
            if (!found) {
                // No high-priority packet found, process low-priority
                if (!input_ports[i].is_empty()) {
                    Packet pkt = input_ports[i].dequeue();
                    pkt.start_service_time = current_time;
                    if (output_ports[pkt.dest_port].enqueue(pkt)) {
                        // Packet enqueued to output port
                    } else {
                        // Output buffer full, packet dropped
                    }
                }
            }
        }
    }
    // Process output ports (departure)
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!output_ports[i].is_empty()) {
            Packet pkt = output_ports[i].dequeue();
            pkt.finish_time = current_time;
            departed_packets.push_back(pkt);
        }
    }
}

// WFQ Scheduler Implementation
WFQScheduler::WFQScheduler() {
    virtual_time.resize(NUM_PORTS, 0.0);
    finish_time.resize(NUM_PORTS, 0.0);
}

void WFQScheduler::schedule(vector<Port>& input_ports,
                            vector<Port>& output_ports,
                            int current_time,
                            vector<Packet>& departed_packets) {
    double weight[NUM_PORTS];
    for (int i = 0; i < NUM_PORTS; ++i) {
        weight[i] = 1.0; // Equal weights; adjust as needed
    }
    // Calculate finish times
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!input_ports[i].is_empty()) {
            Packet pkt = input_ports[i].buffer.front();
            finish_time[i] = max(virtual_time[i], (double)pkt.arrival_time) + pkt.size / weight[i];
        }
    }
    // Find packet with minimum finish time
    int min_idx = -1;
    double min_finish_time = 1e9;
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!input_ports[i].is_empty() && finish_time[i] < min_finish_time) {
            min_finish_time = finish_time[i];
            min_idx = i;
        }
    }
    if (min_idx != -1) {
        Packet pkt = input_ports[min_idx].dequeue();
        virtual_time[min_idx] = finish_time[min_idx];
        pkt.start_service_time = current_time;
        if (output_ports[pkt.dest_port].enqueue(pkt)) {
            // Packet enqueued
        } else {
            // Output buffer full, packet dropped
        }
    }
    // Process output ports (departure)
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!output_ports[i].is_empty()) {
            Packet pkt = output_ports[i].dequeue();
            pkt.finish_time = current_time;
            departed_packets.push_back(pkt);
        }
    }
}

// Round Robin Scheduler Implementation
RRScheduler::RRScheduler() {
    last_served_input = -1;
}

void RRScheduler::schedule(vector<Port>& input_ports,
                           vector<Port>& output_ports,
                           int current_time,
                           vector<Packet>& departed_packets) {
    int start = (last_served_input + 1) % NUM_PORTS;
    for (int i = 0; i < NUM_PORTS; ++i) {
        int idx = (start + i) % NUM_PORTS;
        if (!input_ports[idx].is_empty()) {
            Packet pkt = input_ports[idx].dequeue();
            pkt.start_service_time = current_time;
            if (output_ports[pkt.dest_port].enqueue(pkt)) {
                // Packet enqueued
            } else {
                // Output buffer full, packet dropped
            }
            last_served_input = idx;
            break;
        }
    }
    // Process output ports (departure)
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!output_ports[i].is_empty()) {
            Packet pkt = output_ports[i].dequeue();
            pkt.finish_time = current_time;
            departed_packets.push_back(pkt);
        }
    }
}

// iSLIP Scheduler Implementation
iSLIPScheduler::iSLIPScheduler() {
    input_pointers.resize(NUM_PORTS, 0);
    output_pointers.resize(NUM_PORTS, 0);
}

void iSLIPScheduler::schedule(vector<Port>& input_ports,
                              vector<Port>& output_ports,
                              int current_time,
                              vector<Packet>& departed_packets) {
    vector<vector<int>> requests(NUM_PORTS);
    vector<int> grants(NUM_PORTS, -1);
    vector<int> accepts(NUM_PORTS, -1);

    // Request phase
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!input_ports[i].is_empty()) {
            Packet pkt = input_ports[i].buffer.front();
            requests[pkt.dest_port].push_back(i);
        }
    }

    // Grant phase
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!requests[i].empty()) {
            int start = output_pointers[i];
            for (int j = 0; j < requests[i].size(); ++j) {
                int idx = (start + j) % requests[i].size();
                int input_idx = requests[i][idx];
                if (grants[input_idx] == -1) {
                    grants[input_idx] = i;
                    break;
                }
            }
        }
    }

    // Accept phase
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (grants[i] != -1) {
            int output_idx = grants[i];
            if (accepts[output_idx] == -1) {
                accepts[output_idx] = i;
                // Update pointers
                input_pointers[i] = (input_pointers[i] + 1) % NUM_PORTS;
                output_pointers[output_idx] = (output_pointers[output_idx] + 1) % NUM_PORTS;

                // Move packet
                Packet pkt = input_ports[i].dequeue();
                pkt.start_service_time = current_time;
                if (output_ports[output_idx].enqueue(pkt)) {
                    // Packet enqueued
                } else {
                    // Output buffer full, packet dropped
                }
            }
        }
    }

    // Process output ports (departure)
    for (int i = 0; i < NUM_PORTS; ++i) {
        if (!output_ports[i].is_empty()) {
            Packet pkt = output_ports[i].dequeue();
            pkt.finish_time = current_time;
            departed_packets.push_back(pkt);
        }
    }
}
