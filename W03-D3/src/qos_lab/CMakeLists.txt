cmake_minimum_required(VERSION 3.8)
project(qos_lab)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(my_interfaces REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(qos_publisher src/qos_publisher.cpp)
ament_target_dependencies(qos_publisher rclcpp my_interfaces)

add_executable(qos_subscriber src/qos_subscriber.cpp)
ament_target_dependencies(qos_subscriber rclcpp my_interfaces)

install(TARGETS
  qos_publisher
  qos_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()