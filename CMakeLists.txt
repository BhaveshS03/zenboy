
# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ZenBoyEmulator VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the source and header file paths
set(SOURCES
    main.cpp
    lib/cart.cpp
    lib/emu.cpp
    lib/bus.cpp
    lib/cpu.cpp
    lib/timer.cpp
    lib/instructions.cpp
    lib/operations.cpp
)

set(HEADERS
    headers/cart.hpp
    headers/common.hpp
    headers/emu.hpp
    headers/cpu.hpp
    headers/bus.hpp
    headers/timer.hpp
    headers/instructions.hpp
)

# Add the executable
add_executable(emulator ${SOURCES} ${HEADERS})

# Include the headers directory for header file resolution
target_include_directories(emulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)
