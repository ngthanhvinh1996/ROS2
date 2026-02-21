cmake_minimum_required(VERSION 3.8)
project(robot_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(std_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)

find_package(mipi_cam_local REQUIRED) 
# -----------------------------------------

include_directories(
  include
  include/robot_camera
  ${mipi_cam_local_INCLUDE_DIRS}
  /usr/include/jsoncpp 
  /usr/include/hobot 
  /usr/include/vio
)

link_directories(
  /usr/lib
  /usr/lib/aarch64-linux-gnu
  /usr/hobot/lib
  /usr/lib/hbmedia
  /usr/lib/system/lib
)

add_executable(robot_camera
  src/main.cpp
  src/robot_camera_node.cpp
  src/robot_camera.cpp
)

ament_target_dependencies(robot_camera
  rclcpp
  sensor_msgs
  cv_bridge
  mipi_cam_local 
)

target_link_libraries(robot_camera
  ${OpenCV_LIBRARIES}
  yaml-cpp
  jsoncpp
  hbmem
  multimedia
  cam
  vpf
  Nano2D
  Nano2Dutil
)

install(TARGETS
  robot_camera
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()