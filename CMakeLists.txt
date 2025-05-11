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

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# === Test Executable ===
add_executable(test_events tests/test_events.cpp)
target_include_directories(test_events PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_events pthread ${GTEST_LIBRARIES})

add_executable(test_events_with_channel tests/test_events_with_channel.cpp)
target_include_directories(test_events_with_channel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_events_with_channel pthread ${GTEST_LIBRARIES})

add_executable(channel_test tests/channel_test.cpp)
target_include_directories(channel_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(channel_test ${GTEST_LIBRARIES})

add_executable(move_copy_test tests/move_copy_test.cpp)
target_include_directories(move_copy_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(move_copy_test ${GTEST_LIBRARIES})
