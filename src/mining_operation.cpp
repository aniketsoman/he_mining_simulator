#include <iostream>
#include <string>

#include <chrono>

#include "mining_operation.h"

MiningOperation::MiningOperation(uint numTrucks, uint numUnloadSites) : bRun(false){
    truckSelector = std::make_shared<TruckSelector>();
    unloadSiteSelector = std::make_shared<UnloadSiteSelector>();

    for (uint i = 0; i < numTrucks; ++i) {
        truckSelector->addTruck(i, unloadSiteSelector);
    }

    for (uint i = 0; i < numUnloadSites; ++i) {
        unloadSiteSelector->addUnloadSite(i, truckSelector);
    }
}

MiningOperation::~MiningOperation() {
    this->stop();
}

void MiningOperation::start(uint speed) {
    if (!bRun) {
        std::cout << "Starting Mining operation with " 
                << truckSelector->numTrucks() << " trucks, and "
                << unloadSiteSelector->numUnloadSites() << " unload sites.\n";
        // Additional logic to start the operation would go here
        bRun = true;

        unloadSiteSelector->startSites(speed);

        operationThread = std::thread([this, speed]() { 
            // It's better to check for flag at every blocking step to be as responsive as possible
            while (bRun) {
                auto truck = truckSelector->getAvailableTruck();
                if (bRun && truck) {
                    uint truckId = truck->id();
                    ongoingMiningOps[truckId] = truck->mineLoadTravel(speed);
                }
            }
        });

        std::cout << "Mining operation started.\n";
    }
}

void MiningOperation::stop() {
    if (bRun) {
        std::cout << "Stopping mining operation...\n";
        bRun = false;
        // unblock the operation thread if waiting on empty queue
        truckSelector->truckAvailableNow(0);
        if (operationThread.joinable()) {
            operationThread.join();
        }
        
        // Signal all unload sites to wind down & stop processing 
        // before we wait & may be blocked for mining operation thread to finish
        unloadSiteSelector->stopSites();

        std::cout << "Mining operation stopped.\n";
    }
}

/*
    UnloadSite implementation
*/
UnloadSite::~UnloadSite() {
    this->stop();
}

void UnloadSite::start(uint speed) {
    if (!bRun) {
        std::cout << "Starting Unload Site (" << siteId << ") ...\n";
        bRun = true;
        ulSiteThread = std::thread([this, speed] () {
            while (bRun) {
                uint truckId;
                waitingTrucks.wait_and_pop(truckId);
                
                // Simulate unload operation
                if (bRun) {
                    if (truckSelector) {
                        auto truck = truckSelector->getTruck(truckId);
                        if (truck) {
                            truck->unload(speed); // unload
                            //truck will be available after it travels back.
                            truckSelector->truckAvailable(truck->travelBack(speed));
                        }
                    }
                }
            }
        });
        std::cout << "Unload Site (" << siteId << ") started.\n";
    }
}

void UnloadSite::stop() {
    if (bRun) {
        std::cout << "Stopping Unload Site (" << siteId << ") ...\n";
        bRun = false;
        waitingTrucks.push(0);
        if (ulSiteThread.joinable()) {
            ulSiteThread.join();
        }

        std::cout << "Unload Site (" << siteId << ") stopped.\n";
    }
}

/*
    Truck implementation
*/
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
TruckStatus Truck::mineLoadTravel(uint speed) {
    return std::async(std::launch::async, [this, speed]() {
        // Random mining time between 1 and MAX_MINING_TIME
        srand(time(NULL));
        uint miningTime = 1 + rand() / ((RAND_MAX + 1u) / MAX_MINING_TIME); 
        miningTime *= MINUTES_IN_HOUR / speed; // convert to minutes
        std::this_thread::sleep_for(std::chrono::minutes(miningTime));
        
        // Get the unload site with the smallest queue
        if (siteSelector) {
            // This is not the best of the logic, but it's simple enough for demo
            auto bestSite = siteSelector->popTopUnloadSite();
            // Simulate transit operation
            std::this_thread::sleep_for(std::chrono::minutes(TRANSIT_TIME) / speed);
            // After transit, send it to the unload site chosen
            bestSite->addTruck(trkId);
            // Reinsert the unload site with updated queue size
            siteSelector->pushUnloadSite(bestSite->id(), bestSite->getQueueSize());
        }

        return trkId;
    });
}

void Truck::unload(uint speed) {
    std::this_thread::sleep_for(std::chrono::minutes(UNLOAD_TIME) / speed);
}

TruckStatus Truck::travelBack(uint speed) {
    return std::async(std::launch::async, [this, speed] () {
        std::this_thread::sleep_for(std::chrono::minutes(TRANSIT_TIME) / speed);
        return trkId;
    });
}

/*
    TruckSelector implementation
*/
void TruckSelector::addTruck(uint truckId, UnloadSiteSelectorPtr ulSiteSelector) {
    trucks.push_back(std::make_shared<Truck>(truckId, ulSiteSelector));
    truckAvailableNow(truckId);
}

TruckPtr TruckSelector::getTruck(uint truckId){
    return truckId < trucks.size() ? trucks[truckId] : nullptr;
}

// To keep it simple enough, we are not checking if the id is already in queue
void TruckSelector::truckAvailableNow(uint truckId) {
    if (truckId < trucks.size()) {
        std::promise<uint> truckPromise;
        truckPromise.set_value(truckId);
        truckAvailable(truckPromise.get_future());
    }
}

void TruckSelector::truckAvailable(TruckStatus&& truckStatus) {
    availableTrucks.push(std::move(truckStatus));
}

TruckPtr TruckSelector::getAvailableTruck() {
    TruckStatus truckStatus;
    availableTrucks.wait_and_pop(truckStatus);
    uint truckId = truckStatus.get();
    return truckId < trucks.size() ? trucks[truckId] : nullptr;
}

/*
    UnloadSiteSelector implementation
*/
void UnloadSiteSelector::addUnloadSite(uint siteId, TruckSelectorPtr truckSelector) {
    unloadSites.push_back(std::make_shared<UnloadSite>(siteId, truckSelector));
    pushUnloadSite(siteId, 0);
}

void UnloadSiteSelector::startSites(uint speed) {
    for (auto site : unloadSites) {
        site->start(speed);
    }
}

void UnloadSiteSelector::stopSites() {
    for (auto site : unloadSites) {
        site->stop();
    }
}

// To keep it simple enough, we are not checking if the id is already in queue
void UnloadSiteSelector::pushUnloadSite(uint siteId, uint qSize) {
    std::unique_lock<std::shared_mutex> lock {pqMutex};
    if (siteId < unloadSites.size()) {
        pqUnloadSites.push({qSize, siteId});
    }
}

UnloadSitePtr UnloadSiteSelector::topUnloadSite() {
    std::shared_lock<std::shared_mutex> lock {pqMutex};
    uint siteId = pqUnloadSites.top().second;
    return unloadSites[siteId];
}

UnloadSitePtr UnloadSiteSelector::popTopUnloadSite() {
    std::unique_lock<std::shared_mutex> lock {pqMutex};
    uint siteId = pqUnloadSites.top().second;
    pqUnloadSites.pop();
    return unloadSites[siteId];
}