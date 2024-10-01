#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "packet.h"
#include "port.h"
#include "scheduler.h"

#define NUM_PORTS 8
#define SIMULATION_TIME 10000

using namespace std;

// Global metrics
struct Metrics {
    int total_packets_generated = 0;
    int total_packets_processed = 0;
    int total_packets_dropped = 0;
    double total_waiting_time = 0.0;
    double total_turnaround_time = 0.0;
    vector<int> buffer_occupancy;
    vector<int> packets_dropped_per_port;
    Metrics() {
        buffer_occupancy.resize(NUM_PORTS, 0);
        packets_dropped_per_port.resize(NUM_PORTS, 0);
    }
};

void generate_traffic(vector<Port>& input_ports, int current_time, Metrics& metrics, int traffic_type) {
    default_random_engine generator(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> dest_dist(0, NUM_PORTS - 1);
    bernoulli_distribution arrival_dist(0.5); // Adjust arrival rate as needed
    bernoulli_distribution priority_dist(0.2); // 20% high priority
    bernoulli_distribution burst_dist(0.1); // 10% chance of burst

    for (int i = 0; i < NUM_PORTS; ++i) {
        bool packet_arrives = arrival_dist(generator);
        if (packet_arrives || (traffic_type == 3 && burst_dist(generator))) {
            Packet pkt;
            pkt.id = metrics.total_packets_generated++;
            pkt.source_port = i;
            pkt.dest_port = dest_dist(generator);
            pkt.arrival_time = current_time;
            pkt.size = 1; // Fixed size
            pkt.priority = 0; // Low priority

            if (traffic_type == 2) {
                // Non-uniform traffic
                if (i < NUM_PORTS / 2) {
                    pkt.priority = 1; // High priority
                }
            } else if (traffic_type == 1) {
                // Uniform traffic
                pkt.priority = priority_dist(generator);
            } else if (traffic_type == 3) {
                // Bursty traffic
                pkt.priority = priority_dist(generator);
            }

            if (!input_ports[i].enqueue(pkt)) {
                // Packet dropped due to input buffer overflow
                metrics.total_packets_dropped++;
                metrics.packets_dropped_per_port[i]++;
            }
        }
    }
}

int main() {
    // Initialize input and output ports
    vector<Port> input_ports(NUM_PORTS);
    vector<Port> output_ports(NUM_PORTS);

    // Choose scheduler
    Scheduler* scheduler;
    int scheduler_choice;
    cout << "Select Scheduler:\n1. Priority Scheduling\n2. Weighted Fair Queuing\n3. Round Robin\n4. iSLIP\n";
    cin >> scheduler_choice;
    switch (scheduler_choice) {
        case 1:
            scheduler = new PriorityScheduler();
            break;
        case 2:
            scheduler = new WFQScheduler();
            break;
        case 3:
            scheduler = new RRScheduler();
            break;
        case 4:
            scheduler = new iSLIPScheduler();
            break;
        default:
            cout << "Invalid choice. Exiting.\n";
            return 0;
    }

    // Choose traffic type
    int traffic_type;
    cout << "Select Traffic Type:\n1. Uniform Traffic\n2. Non-Uniform Traffic\n3. Bursty Traffic\n";
    cin >> traffic_type;

    Metrics metrics;
    vector<Packet> departed_packets;

    // Simulation loop
    for (int time = 0; time < SIMULATION_TIME; ++time) {
        // Generate traffic
        generate_traffic(input_ports, time, metrics, traffic_type);

        // Schedule packets
        scheduler->schedule(input_ports, output_ports, time, departed_packets);

        // Update metrics
        for (int i = 0; i < NUM_PORTS; ++i) {
            metrics.buffer_occupancy[i] += input_ports[i].size();
        }
    }

    // Calculate metrics
    for (auto& pkt : departed_packets) {
        double waiting_time = pkt.start_service_time - pkt.arrival_time;
        double turnaround_time = pkt.finish_time - pkt.arrival_time;
        metrics.total_waiting_time += waiting_time;
        metrics.total_turnaround_time += turnaround_time;
    }
    metrics.total_packets_processed = departed_packets.size();

    // Display metrics
    cout << "\n--- Simulation Results ---\n";
    cout << "Total Packets Generated: " << metrics.total_packets_generated << endl;
    cout << "Total Packets Processed: " << metrics.total_packets_processed << endl;
    cout << "Total Packets Dropped: " << metrics.total_packets_dropped << endl;
    cout << "Average Waiting Time: " << metrics.total_waiting_time / metrics.total_packets_processed << endl;
    cout << "Average Turnaround Time: " << metrics.total_turnaround_time / metrics.total_packets_processed << endl;
    cout << "Average Buffer Occupancy per Port:\n";
    for (int i = 0; i < NUM_PORTS; ++i) {
        cout << "Port " << i << ": " << metrics.buffer_occupancy[i] / (double)SIMULATION_TIME << endl;
    }
    cout << "Packet Drop Rate per Port:\n";
    for (int i = 0; i < NUM_PORTS; ++i) {
        cout << "Port " << i << ": " << (metrics.packets_dropped_per_port[i] / (double)metrics.total_packets_generated) * 100 << "%\n";
    }

    delete scheduler;
    return 0;
}
