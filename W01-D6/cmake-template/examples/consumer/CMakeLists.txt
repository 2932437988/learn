cmake_minimum_required(VERSION 3.22)
project(rt_consumer LANGUAGES CXX)

find_package(robot_template 0.1 REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE robot_template::core)