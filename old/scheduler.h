#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "port.h"

#define NUM_PORTS 8

class Scheduler {
public:
    virtual void schedule(std::vector<Port>& input_ports,
                          std::vector<Port>& output_ports,
                          int current_time,
                          std::vector<Packet>& departed_packets) = 0;
};

class PriorityScheduler : public Scheduler {
public:
    void schedule(std::vector<Port>& input_ports,
                  std::vector<Port>& output_ports,
                  int current_time,
                  std::vector<Packet>& departed_packets) override;
};

class WFQScheduler : public Scheduler {
private:
    std::vector<double> virtual_time;
    std::vector<double> finish_time;
public:
    WFQScheduler();
    void schedule(std::vector<Port>& input_ports,
                  std::vector<Port>& output_ports,
                  int current_time,
                  std::vector<Packet>& departed_packets) override;
};

class RRScheduler : public Scheduler {
private:
    int last_served_input;
public:
    RRScheduler();
    void schedule(std::vector<Port>& input_ports,
                  std::vector<Port>& output_ports,
                  int current_time,
                  std::vector<Packet>& departed_packets) override;
};

class iSLIPScheduler : public Scheduler {
private:
    std::vector<int> input_pointers;
    std::vector<int> output_pointers;
public:
    iSLIPScheduler();
    void schedule(std::vector<Port>& input_ports,
                  std::vector<Port>& output_ports,
                  int current_time,
                  std::vector<Packet>& departed_packets) override;
};

#endif // SCHEDULER_H
