#pragma once
#include <unordered_map>
#include "common.h"

class Simulator {
public:
    Simulator(size_t trucks, size_t unloadSites, size_t speed = DEFAULT_SPEED, 
        size_t time = DEFAULT_SIMULATION_TIME);
    ~Simulator() = default;
    void run();
    void printResults();

    // Setters for optional parameters
    inline void simulationSpeed(size_t speed) {
        _simulationSpeed = speed;
    }
    inline void simulationTime(size_t time) {
        _simulationTime = time;
    }

private:
    size_t _numTrucks;
    size_t _numUnloadSites;
    /*
        Simulation speed
        1  - run in real-time,
        60 - run in simulationTime minutes,
        3600 - run in simulationTime seconds
    */
    size_t _simulationSpeed;
    size_t _simulationTime; // in hours - default 72 hours
};