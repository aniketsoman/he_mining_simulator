#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "mining_operation.h"

MiningOperation::MiningOperation(size_t numTrucks, size_t numUnloadSites) : _bRun(false){
    for (size_t i = 1; i <= numTrucks; ++i) {
        _trucks.emplace_back(i);
        _availableTrucks.push(i);
    }

    for (size_t i = 1; i <= numUnloadSites; ++i) {
        _unloadSites.push_back(std::make_shared<UnloadSite>(i));
        _unload_site_availability.push({0, i}); // initial queue size is 0
    }
}

void MiningOperation::start(size_t speed) {
    std::cout << "Starting Mining operation with " 
              //<< _mines.size() << " mines, "
              << _trucks.size() << " trucks, and "
              << _unloadSites.size() << " unload sites.\n";
    // Additional logic to start the operation would go here
    _bRun = true;

    for (auto site : _unloadSites) {
        site->start(speed);
    }

    _operationThread = std::thread([this, speed]() { 
        // It's better to check for flag at every blocking step to be as responsive as possible
        while (_bRun) {
            size_t truckId;
            _availableTrucks.wait_and_pop(truckId);

            if (_ongoingMiningOps.count(truckId) > 0) {
                auto& fut = _ongoingMiningOps[truckId];
                if (fut.valid()) {
                    fut.get();
                }
                _ongoingMiningOps.erase(truckId);
            }

            if (_bRun) {
                _ongoingMiningOps[truckId] = mineAndDispatch(truckId, speed);
            } else { // Stop Requested, clean up
                for (auto& [truckId, fut] : _ongoingMiningOps) {
                    if (fut.valid()) {
                        fut.get(); // wait for the mining operation to complete
                    }
                }
            }
            
            // Check for empty trucks at the unload sites & keep going
            // If stop requested, this cleans up anyways
            for (auto uls : _unloadSites) { 
                auto truckFutPtr = uls->popEmptyTruck();
                while (truckFutPtr != nullptr) {
                    auto truckId = truckFutPtr->get();
                    if (_bRun) {
                        _availableTrucks.push(truckId);
                    }
                    truckFutPtr = uls->popEmptyTruck();
                }
            }
        }
    });

    std::cout << "Mining operation started.\n";
}

void MiningOperation::stop() {
    std::cout << "Stopping mining operation...\n";
    _bRun = false;
    _availableTrucks.push(0); // unblock the operation thread if waiting
    
    // Signal all unload sites to wind down & stop processing 
    // before we wait & may be blocked for mining operation thread to finish
    for (auto site : _unloadSites) {
        site->stop();
    }

    _operationThread.join();

    std::cout << "Mining operation stopped.\n";
}

/*
    An attempt to simulate mining process as per requirments.
    Each get access to mine instantly & can start the process 
    independently or in parallel with others in an async task to 
        - Wait random time between 1 to 5 hours mining
        - Pick the site with smallest wait queue
        - Wait in transit for 30 minutes
        - Park at the end of the queue at unload site
    A future is returned to check & wait for the status.
*/
std::future<size_t> MiningOperation::mineAndDispatch(size_t truckId, size_t speed) {
    // Simulate mining operation
    return std::async(std::launch::async, [this,speed, truckId]() {
        // Random mining time between 1 and MAX_MINING_TIME
        srand(time(NULL));
        size_t miningTime = 1 + rand() / ((RAND_MAX + 1u) / MAX_MINING_TIME); 
        miningTime *= MINUTES_IN_HOUR / speed; // convert to minutes
        std::this_thread::sleep_for(std::chrono::minutes(miningTime));
        if (!_unload_site_availability.empty()) {
            // Get the unload site with the smallest queue
            auto [_, bestSiteId] = _unload_site_availability.top();
            _unload_site_availability.pop();
            // Simulate transit operation
            std::this_thread::sleep_for(std::chrono::minutes(TRANSIT_TIME) / speed);
            // After transit, send it to the unload site chosen
            _unloadSites[bestSiteId]->addTruck(truckId);
            // Reinsert the unload site with updated queue size
            _unload_site_availability.push({_unloadSites[bestSiteId]->getQueueSize(), 
                                            bestSiteId});
        }
        return truckId;
    });
}

void UnloadSite::start(size_t speed) {
    std::cout << "Starting Unload Site (" << _id << ") ...\n";
    _bRun = true;
    _ulSiteThread = std::thread([this, speed] () {
        while (_bRun) {
            size_t truckId;
            _waitingTrucks.wait_and_pop(truckId);
            
            // Simulate unload operation
            if (_bRun) {
                std::this_thread::sleep_for(std::chrono::minutes(UNLOAD_TIME) / speed);
            }
            
            // Send empty trucks in transit
            if (_bRun) {
                _emptyTrucks.push(std::async(std::launch::async, [truckId, speed] () {
                    std::this_thread::sleep_for(std::chrono::minutes(TRANSIT_TIME) / speed);
                    return truckId;
                }));
            } 
        }
    });
    std::cout << "Unload Site (" << _id << ") started.\n";
}

void UnloadSite::stop() {
    std::cout << "Stopping Unload Site (" << _id << ") ...\n";
    _bRun = false;
    _waitingTrucks.push(0);
    _ulSiteThread.join();

    std::cout << "Unload Site (" << _id << ") stopped.\n";
}