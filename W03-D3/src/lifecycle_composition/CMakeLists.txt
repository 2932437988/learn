cmake_minimum_required(VERSION 3.8)
project(lifecycle_composition)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(my_interfaces REQUIRED)

# 独立的生命周期节点可执行文件
add_executable(lifecycle_talker src/lifecycle_talker.cpp)
ament_target_dependencies(lifecycle_talker
  rclcpp rclcpp_lifecycle lifecycle_msgs my_interfaces)

# 组件共享库：一个库里注册两个插件
add_library(blob_components SHARED
  src/blob_publisher.cpp
  src/blob_subscriber.cpp)
target_include_directories(blob_components PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(blob_components
  rclcpp rclcpp_components my_interfaces)

rclcpp_components_register_node(blob_components
  PLUGIN "lifecycle_composition::BlobPublisher"
  EXECUTABLE blob_publisher)
rclcpp_components_register_node(blob_components
  PLUGIN "lifecycle_composition::BlobSubscriber"
  EXECUTABLE blob_subscriber)

install(TARGETS lifecycle_talker DESTINATION lib/${PROJECT_NAME})
install(TARGETS blob_components LIBRARY DESTINATION lib)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()