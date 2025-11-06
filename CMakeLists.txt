cmake_minimum_required(VERSION 3.12)
project(he_mining_simulator 
    VERSION 1.0
    DESCRIPTION "Mining Simulator with Vehicle Management"
    LANGUAGES CXX
)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Enable optimization for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Collect all source files
file(GLOB_RECURSE SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})
