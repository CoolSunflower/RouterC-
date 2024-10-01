# Router Switch Fabric Simulation

This project simulates a router switch fabric with multiple scheduling algorithms: Priority Scheduling, Weighted Fair Queuing (WFQ), Round Robin (RR), and iSLIP. The simulation compares these algorithms under various traffic conditions.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Building the Project](#building-the-project)
- [Running the Simulation](#running-the-simulation)
- [Simulation Parameters](#simulation-parameters)
- [Modifying the Code](#modifying-the-code)
- [Understanding the Code](#understanding-the-code)
- [Results and Analysis](#results-and-analysis)
- [Conclusion](#conclusion)

---

## Introduction

The router switch fabric simulation models a network router with:

- **8 Input Ports**
- **8 Output Ports**
- **Variable Packet Arrival Rates**
- **Finite Buffer Size of 64 Packets per Port**

The simulation evaluates the performance of four scheduling algorithms under different traffic scenarios to understand their behavior and efficiency.

## Features

- **Scheduling Algorithms Implemented:**
  - Priority Scheduling
  - Weighted Fair Queuing (WFQ)
  - Round Robin (RR)
  - iSLIP

- **Traffic Types:**
  - Uniform Traffic
  - Non-Uniform Traffic
  - Bursty Traffic

- **Performance Metrics Collected:**
  - Queue Throughput
  - Turnaround Time
  - Waiting Time
  - Buffer Occupancy
  - Packet Drop Rate

## Project Structure

- `main.cpp` - Contains the main simulation loop and traffic generation.
- `packet.h` / `packet.cpp` - Defines the `Packet` structure and related functions.
- `port.h` / `port.cpp` - Implements the `Port` class representing input and output buffers.
- `scheduler.h` / `scheduler.cpp` - Contains implementations of the scheduling algorithms.
- `makefile` - Builds the project.
- `README.md` - Project documentation (this file).

## Requirements

- C++11 or higher compiler (e.g., `g++`, `clang++`)
- `make` utility

## Building the Project

Open a terminal in the project directory and run:

```bash
make
```

This command compiles the source code and generates an executable named router_sim.

## Running the Simulation

Execute the simulation using:

```bash
./router_sim
```

Upon running, the program will prompt you to select the scheduling algorithm and the traffic type:

    Select Scheduler:
        1: Priority Scheduling
        2: Weighted Fair Queuing
        3: Round Robin
        4: iSLIP

    Select Traffic Type:
        1: Uniform Traffic
        2: Non-Uniform Traffic
        3: Bursty Traffic

Enter the corresponding number for your choices.

Example:

```bash
Select Scheduler:
1. Priority Scheduling
2. Weighted Fair Queuing
3. Round Robin
4. iSLIP
> 3

Select Traffic Type:
1. Uniform Traffic
2. Non-Uniform Traffic
3. Bursty Traffic
> 2
```

After the simulation runs, it will display the performance metrics collected.

## Simulation Parameters

    Simulation Time: The simulation runs for 10,000 time units by default.
    Buffer Size: Each port has a buffer size of 64 packets.
    Packet Arrival Rate: Adjustable within the generate_traffic function in main.cpp.
    Packet Priorities: High priority (1) and low priority (0).

You can modify these parameters in the code to experiment with different scenarios.

## Modifying the Code

To adjust the simulation parameters or behavior:

    Change Simulation Time:

    In main.cpp, modify the SIMULATION_TIME constant.

    cpp

#define SIMULATION_TIME 10000  // Adjust as needed

Adjust Packet Arrival Rate:

In the generate_traffic function, modify the arrival_dist probability.

cpp

bernoulli_distribution arrival_dist(0.5); // Adjust arrival rate (0.0 to 1.0)

Change Buffer Size:

In port.h, modify the BUFFER_SIZE constant.

cpp

#define BUFFER_SIZE 64  // Adjust buffer size

Modify Weights in WFQ:

In WFQScheduler::schedule, adjust the weight array to assign different weights to ports.

cpp

    weight[i] = 1.0; // Assign weights to ports

## Understanding the Code
### Main Components

    main.cpp: Orchestrates the simulation, handles user input, generates traffic, and collects metrics.
    packet.h / packet.cpp: Defines the Packet structure with attributes like ID, priority, arrival time, etc.
    port.h / port.cpp: Implements the Port class with methods to enqueue and dequeue packets, and track buffer occupancy.
    scheduler.h / scheduler.cpp: Contains the base Scheduler class and derived classes for each scheduling algorithm, implementing the schedule method.

### Scheduling Algorithms

    Priority Scheduling:
        High-priority packets are scheduled before low-priority packets.
        Within the same priority level, packets are served in FIFO order.

    Weighted Fair Queuing (WFQ):
        Assigns weights to each flow or queue.
        Schedules packets based on calculated finish times considering the weights.

    Round Robin (RR):
        Serves each input port in a cyclic order.
        Ensures equal opportunity for all ports to send packets.

    iSLIP:
        An advanced round-robin algorithm.
        Uses input and output pointers and multiple phases (request, grant, accept) to efficiently match inputs to outputs.

## Results and Analysis

After running the simulation, the program outputs:

    Total Packets Generated
    Total Packets Processed
    Total Packets Dropped
    Average Waiting Time
    Average Turnaround Time
    Average Buffer Occupancy per Port
    Packet Drop Rate per Port

We Use these metrics to compare the performance of each scheduling algorithm under different traffic conditions.

## Conclusion

TBD