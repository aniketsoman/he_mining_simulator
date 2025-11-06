Introduction:
  A basic simple single process simulation of a simple logistics scenario 
  of mining & loading, transportation, unloading and back.
  There is a lot to cover & this can be simulated better in a multi process/service
  design that requires more dependencies to build & deploy.
  The structure & design is also (attempted) to be kept simple in 3-4 parts; simulator,
  mining operation entities, other common utilities and stats/reporting (which is pending).
  External dependencies are avoided for keeping the set up simple & to the point.
  This simulates trucks waiting independently i.e. in parallel for mining & loading 
  as well as transit and unloading operation happens in serial fashion per unload center.
  
  Pending/ToDo -
    Statistics/Reporting of data & performance of trucks & loading sites.
    Automated unit tests & system tests
    More review for style, clarity, performance as well as may be a new better approach
    More review for eliminating/minimizing timing issues around threads, synchronization & signalling causing crashes sometime.
    More documentation

Build: 
    Make - 
        make -f he_minining_simulator -mk 
        mkbuild/he_mining_simulator <arguments>
        or
        make -f he_minining_simulator -mk ARGS="<arguments>"
    CMake - 
        cmake -B build
        cmake --build build
        build/he_mining_simulator <arguments>
      (Visit CMake documentation for more @ cmake.org)

Usage: he_mining_simulator [options]
Options:
  Required:
    -t, --trucks <number>        Number of trucks
    -s, --unload-sites <number>  Number of unload sites 
  Optional:
    -x, --speed <value>          Simulation speed (1: real-time, 60: minutes, 3600: seconds) (default: 60)
    -t, --time <hours>           Simulation time in hours (default: 72)
    -m, --mines <number>         Number of mines (Not used at present, reserved for future use)
    -h, --help                   Show this help message
