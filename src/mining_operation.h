#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <future>
#include <thread>
#include <shared_mutex>
#include <memory>

#include "common.h"
#include "threadsafe_queue.h"

class UnloadSiteSelector;
class TruckSelector;
class Truck;
class UnloadSite;
using TruckStatus = std::future<uint>;
using UnloadSiteSelectorPtr = std::shared_ptr<UnloadSiteSelector>;
using TruckSelectorPtr = std::shared_ptr<TruckSelector>;
using TruckPtr = std::shared_ptr<Truck>;
using UnloadSitePtr = std::shared_ptr<UnloadSite>;
using SiteQueueSzPair = std::pair<uint, uint>;

class Truck {
public:
    Truck(uint id, UnloadSiteSelectorPtr ulSiteSelector) 
            : trkId(id), siteSelector(ulSiteSelector) {}
    ~Truck() = default;

    uint id() const {
        return trkId;
    }

    TruckStatus mineLoadTravel(uint speed);
    void unload(uint speed);
    TruckStatus travelBack(uint speed);

private:
    uint trkId;
    UnloadSiteSelectorPtr siteSelector;
};

class UnloadSite {
public:
    UnloadSite(uint id, TruckSelectorPtr truckSelector) 
        : siteId(id), bRun(false), truckSelector(truckSelector) {}
    ~UnloadSite();

    uint id() const {
        return siteId;
    }

    size_t getQueueSize() const {
        return waitingTrucks.size();
    }

    void addTruck(uint truckId) {
       waitingTrucks.push(truckId);
    }

    void start(uint speed);
    void stop();

private:
    uint siteId;
    threadsafe_queue<uint>waitingTrucks;
    std::atomic<bool> bRun;
    std::thread ulSiteThread;
    TruckSelectorPtr truckSelector;
};

class TruckSelector {
public:
    TruckSelector() = default;
    // We rely on std::future (TaskStatus) destructor to block for async calls to
    // finish when availableTrucks queue is being destroyed.
    ~TruckSelector() = default; 
   
    void addTruck(uint truckId, UnloadSiteSelectorPtr ulSiteSelector);
    TruckPtr getTruck(uint truckId);
    size_t numTrucks() {return trucks.size();}

    void truckAvailableNow(uint truckId);
    void truckAvailable(TruckStatus&& status);
    TruckPtr getAvailableTruck();

private:
    std::vector<TruckPtr> trucks;
    threadsafe_queue<TruckStatus> availableTrucks;
};

class UnloadSiteSelector {
public:
    UnloadSiteSelector() = default;
    ~UnloadSiteSelector() = default;

    void addUnloadSite(uint siteId, TruckSelectorPtr truckSelector);
    void startSites(uint speed);
    void stopSites();

    // Priority Queue operations
    void pushUnloadSite(uint siteId, uint qSize);
    UnloadSitePtr topUnloadSite();
    UnloadSitePtr popTopUnloadSite();
    size_t numUnloadSites() {return unloadSites.size();}

private:
    std::vector<UnloadSitePtr> unloadSites;
    // Min heap of pair<queue size, unload site id> so we don't need custom comparator
    // std::greater will suffice sorting on queue size 
    std::priority_queue<SiteQueueSzPair, std::vector<SiteQueueSzPair>, 
                        std::greater<SiteQueueSzPair> > pqUnloadSites;
    mutable std::shared_mutex pqMutex;
};

class MiningOperation {
public:
    MiningOperation(uint numTrucks, uint numUnloadSites);
    ~MiningOperation();
    void start(uint speed);
    void stop(); 

private:
    // Save futures so we wait until the unifinished work is done
    std::unordered_map<uint, TruckStatus> ongoingMiningOps;
   
    std::atomic<bool> bRun;
    std::thread operationThread;

    UnloadSiteSelectorPtr unloadSiteSelector;
    TruckSelectorPtr truckSelector;
};