cmake_minimum_required(VERSION 3.10)
project(LimitOrderBookSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable threading
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/Utils.cpp
)

# Executable
add_executable(simulator ${SOURCES})

# Link threading library
target_link_libraries(simulator Threads::Threads)