#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "simulator.h"
#include "mining_operation.h"

Simulator::Simulator(uint trucks, uint unloadSites, uint speed, uint time) 
        : _numTrucks(trucks), _numUnloadSites(unloadSites), 
        simultnSpeed(speed), simultnTime(time) {
    if (_numTrucks == 0 || _numUnloadSites == 0) {
        throw std::invalid_argument(\
            "Number of trucks and unload sites must be greater than zero.");
    }
}

void Simulator::run() {
    std::cout << "Starting simulation with the following parameters:\n";
    std::cout << "Number of Trucks: " << _numTrucks << "\n";
    std::cout << "Number of Unload Sites: " << _numUnloadSites << "\n";
    std::cout << "Simulation Time (hours): " << simultnTime << "\n";
    std::cout << "Simulation Speed: " << simultnSpeed << "\n";

    // Simulation loop would go here
    MiningOperation mOp(_numTrucks, _numUnloadSites);
    std::cout << "Starting Simulation...\n";
    mOp.start(simultnSpeed);
    std::cout << "Simulation started.\n";
    while (true) {
       // sleep for simultnTime / simultnSpeed time
        std::this_thread::sleep_for(std::chrono::minutes(simultnTime * MINUTES_IN_HOUR 
                                                        / simultnSpeed));
        std::cout << "Stopping simulation...\n";
        mOp.stop(); // Should block until mining operation wraps up current tasks & cleans up
        break;
    }
    std::cout << "Simulation stopped.\n";
}

void Simulator::printResults() {
    std::cout << "Simulation completed. Printing results...\n";
    // Print simulation results here
}

int main(int argc, char** argv) {
    std::string programName = argv[0];
    programName = programName.substr(programName.find_last_of("/\\") + 1);

    const auto printUsage = [&programName]() {
        std::cout << "Usage: "<<  programName << " [options]\n";
        std::cout << "Options:\n";
        std::cout << "  Required:\n";
        std::cout << "    -t, --trucks <number>        Number of trucks\n";
        std::cout << "    -s, --unload-sites <number>  Number of unload sites\n";
        std::cout << "  Optional:\n";
        std::cout << "    -x, --speed <value>          Simulation speed (1: real-time, 60: minutes, 3600: seconds) (default: 60)\n";
        std::cout << "    -T, --time <hours>           Simulation time in hours (default: 72)\n";
        std::cout << "    -h, --help                   Show this help message\n";
    };

    const auto errorExit = [&printUsage](const std::string& message) {
        std::cerr << "Error: " << message << "\n";
        printUsage();
        exit(1);
    };

     // Ideally we should use a command-line parsing library rather than reinventing it, 
    // but to avoid extra dependencies, here is a simple implementation.
    const auto argParser = [&printUsage, &errorExit](int argc, char** argv) {
        std::unordered_map<std::string, uint> args;
        // List & mapping of valid arguments
        std::unordered_map<std::string, std::string> validArgs = {
            {"-t", "numTrucks"}, {"-trucks", "numTrucks"},
            {"-s", "numUnloadSites"}, {"--unload-sites", "numUnloadSites"},
            {"-x", "simulationSpeed"}, {"--speed", "simulationSpeed"},
            {"-T", "simulationTime"}, {"--time", "simulationTime"}
        };

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            // Check if argument is valid
            if (validArgs.count(arg) == 0) {
                if (arg == "--help" || arg == "-h") {
                    printUsage();
                    exit(0);
                } 
                errorExit("Error: Unknown argument " + arg);
            }

            // Check for duplicate arguments
            if (args.count(arg) > 0) {
                errorExit("Error: Duplicate argument " + arg) ;
            }

            // Check if there is a value following the argument
            if (++i >= argc) {
                errorExit("Error: Missing value for argument " + arg);
            }

            uint val = 0;
            try {
                val = std::stoul(argv[i]);
            } catch (const std::invalid_argument& e) {
                errorExit("Error: Invalid value for argument " + arg + ": " + argv[i]);
            } catch (const std::out_of_range& e) {
                errorExit("Error: Value out of range for argument " + arg + ": " + argv[i]);
            }

            args[validArgs[arg]] = val; 
        }
        return args;
    };

    auto args = argParser(argc, argv);
    try {
        Simulator sim(args["numTrucks"], args["numUnloadSites"]);
        if (args.count("simulationSpeed") > 0) {
            sim.simulationSpeed(args["simulationSpeed"]);
        }
        if (args.count("simulationTime") > 0) {
            sim.simulationTime(args["simulationTime"]);
        }
        sim.run();
        sim.printResults();
    } catch (const std::invalid_argument& e) {
        errorExit(e.what());
    }

    return 0;
}

