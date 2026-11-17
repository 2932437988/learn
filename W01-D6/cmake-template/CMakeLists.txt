cmake_minimum_required(VERSION 3.22)
project(robot_template VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)                       # 定义 BUILD_TESTING (默认 ON) 并调用 enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---------- 依赖：进导出包的三个，一律 find_package ----------
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

option(USE_SYSTEM_GTEST "用系统 GTest；OFF 则用 FetchContent 锁版本" ON)
if(BUILD_TESTING)
  if(USE_SYSTEM_GTEST)
    find_package(GTest REQUIRED)
  else()
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

# ---------- 构建矩阵（沿用 D4 1.4 的 ENABLE_TSAN 并扩展） ----------
option(ENABLE_ASAN  "AddressSanitizer" OFF)
option(ENABLE_UBSAN "UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN  "ThreadSanitizer" OFF)
if(ENABLE_ASAN AND ENABLE_TSAN)
  message(FATAL_ERROR "ASan 与 TSan 的 runtime 冲突，只能开一个（见 D4 7.7）")
endif()

# sanitizer 必须对**所有目标**统一插桩（含 tests/ 子目录里的测试目标）。
# 用目录级选项，且必须在任何 add_library / add_executable / add_subdirectory 之前设置；
# 目录属性不由目标携带，不会写进 install(EXPORT) 的导出文件。
if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address)
elseif(ENABLE_UBSAN)
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=undefined)
elseif(ENABLE_TSAN)
  add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g -O1)
  add_link_options(-fsanitize=thread)
endif()

function(rt_apply_warnings tgt)
  target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
endfunction()

# ---------- 库 ----------
add_library(robot_template src/config.cpp src/log.cpp)   # 不写类型：由 BUILD_SHARED_LIBS 决定（默认 STATIC）
add_library(robot_template::core ALIAS robot_template)   # 项目内部别名

set_target_properties(robot_template PROPERTIES
  EXPORT_NAME core
  POSITION_INDEPENDENT_CODE ON
)

target_compile_features(robot_template PUBLIC cxx_std_17)
target_include_directories(robot_template
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(robot_template
  PUBLIC  Threads::Threads
  PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog
)

add_executable(demo apps/demo.cpp)
target_link_libraries(demo PRIVATE robot_template::core)

# 顺序：目标必须先存在，才能给它加编译选项
foreach(tgt robot_template demo)
  rt_apply_warnings(${tgt})
endforeach()

# ---------- 测试 ----------
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# ---------- install / export（第 6 节逐行讲解） ----------
install(TARGETS robot_template
  EXPORT robot_templateTargets
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT robot_templateTargets
  FILE robot_templateTargets.cmake
  NAMESPACE robot_template::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/robot_template
)

configure_package_config_file(
  cmake/robot_templateConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/robot_templateConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/robot_template
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/robot_templateConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/robot_templateConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/robot_templateConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/robot_template
)
