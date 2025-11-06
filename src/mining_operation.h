#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <future>
#include <memory>

#include "common.h"
#include "threadsafe_queue.h"

class Truck {
public:
    Truck(size_t id) : _id(id) {}
    ~Truck() = default;

    size_t id() const {
        return _id;
    }
private:
    size_t _id;
};

class UnloadSite {
public:
    UnloadSite(size_t id) : _id(id), _bRun(false) {}
    ~UnloadSite() = default;

    size_t id() const {
        return _id;
    }

    size_t getQueueSize() const {
        return _waitingTrucks.size();
    }

    void addTruck(size_t truckId) {
        _waitingTrucks.push(truckId);
    }

    std::shared_ptr<std::future<size_t>> popEmptyTruck() {
        return _emptyTrucks.try_pop();
    }

    void start(size_t speed);
    void stop();

private:
    size_t _id;
    threadsafe_queue<size_t> _waitingTrucks;
    threadsafe_queue<std::future<size_t>> _emptyTrucks;
    std::atomic<bool> _bRun;
    std::thread _ulSiteThread;
};

class MiningOperation {
public:
    MiningOperation(size_t numTrucks, size_t numUnloadSites);
    ~MiningOperation() = default;
    void start(size_t speed);
    void stop(); 

private:
    std::vector<Truck> _trucks;
    std::vector<std::shared_ptr<UnloadSite>> _unloadSites;
    // Save futures so we wait until the unifinished work is done
    std::unordered_map<size_t, std::future<size_t>> _ongoingMiningOps;
    // Min heap of pair<queue size, unload site id> so we don't need custom comparator
    // std::greater will suffice
    std::priority_queue<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>, 
                        std::greater<std::pair<size_t, size_t>>> _unload_site_availability;
    
    std::atomic<bool> _bRun;
    std::thread _operationThread;
    threadsafe_queue<size_t> _availableTrucks;

    std::future<size_t> mineAndDispatch(size_t truckId, size_t speed); // Simulate mining + dispatching to unload site
};