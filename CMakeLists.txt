cmake_minimum_required(VERSION 3.10)
project(oska LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# === Test Executable ===
add_executable(testEvents test/testEvents.cpp)
target_include_directories(testEvents PRIVATE include)
target_link_libraries(testEvents pthread)
