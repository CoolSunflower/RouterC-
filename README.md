# Router Scheduling Algorithms - Simulation & Comparison

    Adarsh Gupta (220101003) 
    Tanvi Doshi (220101102)
    Vasudha Meena (220101108)
    Yash Jain (220101115)

## Table of Contents
1. [Introduction](#introduction)
2. [Project Structure](#project-structure)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Scheduling Algorithms](#scheduling-algorithms)
6. [Performance Metrics](#performance-metrics)
7. [Simulation Results](#simulation-results)
8. [Conclusion](#conclusion)

---

## Introduction
This project involves the implementation and comparison of several router scheduling algorithms used in network routers with 8 input and output ports. The goal is to design a switch fabric capable of handling high-throughput traffic. Four algorithms are implemented:
1. **Priority Scheduling**
2. **Weighted Fair Queuing (WFQ)**
3. **Round Robin (RR)**
4. **iSLIP**

The project compares these algorithms on metrics like Queue Throughput, Turnaround Time, Waiting Time, Buffer Occupancy, and Packet Drop Rate.

## Project Structure
The repository contains the following files and directories:
- `/SimulationOutput/` - Folder containing simulation results for each scheduling algorithm.
- `/SimulationReports/` - Folder for generated reports after simulations.
- `/Results/` - Contains results for Uniform, Non-Uniform and Bustry Simulation.
- The rest of the files constitute the source code.

## Installation
To set up and run the simulation on your machine, follow these steps:
1. Clone the repository:
   ```bash
   git clone https://github.com/CoolSunflower/RouterC-/
   ```
2. Navigate to the project directory:

    ```bash
    cd RouterC-
3. Compile the source code:

    ```bash
    make

## Usage

Run the program with the following command:

```bash
./router
```

You will be prompted to select the scheduling algorithm for 
simulation:

    Priority Scheduler
    Weighted Fair Scheduler
    Round Robin Scheduler
    iSLIP Scheduler

After the simulation completes, metrics are automatically calculated and stored in the /SimulationReports/ directory.

## Scheduling Algorithms

1. Priority Scheduling

Packets are scheduled based on strict priority, with certain input queues prioritized over others.

2. Weighted Fair Queuing (WFQ)

Allocates bandwidth fairly to input queues based on assigned weights.

3. Round Robin (RR)

Packets are processed cyclically across all queues, ensuring fairness without priority differentiation.

4. iSLIP

Implements a three-phase approach (Request, Grant, Accept) to efficiently handle packet requests and mitigate head-of-line blocking.

## Performance Metrics

The following metrics are calculated to compare the performance of the scheduling algorithms:

    Queue Throughput: The rate at which packets are processed.

    Turnaround Time: Time from when a packet arrives to when it is processed.

    Waiting Time: Time a packet waits in the queue before processing.

    Buffer Occupancy: How full the buffers are over time.
    Packet Drop Rate: The percentage of packets dropped due to full queues.

## Simulation Results

Three traffic conditions were simulated: Uniform, Non-Uniform, and Bursty. The following are the main observations:

    iSLIP achieved the lowest packet delay and highest throughput, particularly in bursty traffic scenarios.

    Weighted Fair Queuing (WFQ) offered the highest fairness.
    Priority Scheduling led to starvation of low-priority traffic under high load.

Detailed results and graphs are available in the /Results/ folder.

## Conclusion

The iSLIP algorithm provided the best overall performance, with low packet delay and high throughput, making it the most suitable for high-throughput router switch fabrics. Detailed conclusions are drawn in the final report, available in the repository.

For more details, visit the full project repository [here](https://github.com/CoolSunflower/RouterC-/), or refer to the complete report: 'Router Scheduling Algorithms: Simulation & Comparision in C++'.