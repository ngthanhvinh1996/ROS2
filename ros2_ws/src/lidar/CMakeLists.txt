cmake_minimum_required(VERSION 3.8)
project(lidar)

# Require C++17 (needed for static inline members in lidar_log.hpp)
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Set build type to Debug
set(CMAKE_BUILD_TYPE Debug)

# Set TSAN flags
# set(TSAN_FLAGS "-g -fsanitize=thread -O1")

# Add TSAN flags to CXX and C flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TSAN_FLAGS}")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TSAN_FLAGS}")

# Add TSAN flags to EXE and SHARED linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TSAN_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${TSAN_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Dependencies ──────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# ── Include directories ──────────────────────────────────────
include_directories(
  include
)

# ── Executable ────────────────────────────────────────────────
add_executable(lidar_node
  src/main.cpp
  src/lidar_ros2.cpp
  src/lidar_driver.cpp
)

ament_target_dependencies(lidar_node
  rclcpp
  sensor_msgs
  std_msgs
)

# ── Install ───────────────────────────────────────────────────
install(TARGETS lidar_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  params
  DESTINATION share/${PROJECT_NAME}
)

# ── Testing ───────────────────────────────────────────────────
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
