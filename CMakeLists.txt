cmake_minimum_required(VERSION 3.10)
project(oska LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) 

# === Disable RTTI ===
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fno-rtti)
elseif(MSVC)
    add_compile_options(/GR-) # MSVC disables RTTI with /GR-
endif()

# === Test Executable ===
add_executable(testEvents test/testEvents.cpp)
target_include_directories(testEvents PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(testEvents pthread)
