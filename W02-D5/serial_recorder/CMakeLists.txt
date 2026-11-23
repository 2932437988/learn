cmake_minimum_required(VERSION 3.16)
project(serial_recorder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_ENTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

include(CTest)

add_library(recorder_core
  src/frame.cpp
  src/recorder.cpp
  src/player.cpp
  src/cli.cpp
)
target_include_directories(recorder_core PUBLIC include)
target_link_libraries(recorder_core
  PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json Threads::Threads
)

add_executable(serial_recorder src/main.cpp)
target_link_libraries(serial_recorder PRIVATE recorder_core)

if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  include(GoogleTest)
  foreach(t test_frame test_ring_buffer test_player)
    add_executable(${t} tests/${t}.cpp)
    target_link_libraries(${t} PRIVATE recorder_core GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${t})
  endforeach()
endif()