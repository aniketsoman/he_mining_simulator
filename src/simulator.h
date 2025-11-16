#pragma once
#include <unordered_map>
#include "common.h"

class Simulator {
public:
    Simulator(uint trucks, uint unloadSites, uint speed = DEFAULT_SPEED, 
        uint time = DEFAULT_SIMULATION_TIME);
    ~Simulator() = default;
    void run();
    void printResults();

    // Setters for optional parameters
    inline void simulationSpeed(uint speed) {
        simultnSpeed = speed;
    }
    inline void simulationTime(uint time) {
        simultnTime = time;
    }

private:
    uint _numTrucks;
    uint _numUnloadSites;
    /*
        Simulation speed
        1  - run in real-time,
        60 - run in simulationTime minutes,
        3600 - run in simulationTime seconds
    */
    uint simultnSpeed;
    uint simultnTime; // in hours - default 72 hours
};