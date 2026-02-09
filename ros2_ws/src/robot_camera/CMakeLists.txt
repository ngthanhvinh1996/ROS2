cmake_minimum_required(VERSION 3.8)
project(robot_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(mipi_cam_local REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(hbm_img_msgs REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(std_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)

set(HOBOT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../hobot_mipi_cam_local")

include_directories(
  include
  include/robot_camera
  ${HOBOT_SOURCE_DIR}/include
  ${HOBOT_SOURCE_DIR}/src/x5/sensor
  /usr/include/hobot
  /usr/include/vio
)

link_directories(
  /usr/hobot/lib
)

add_executable(robot_camera
  src/robot_camera.cpp
)

ament_target_dependencies(robot_camera
  rclcpp
  mipi_cam_local
  sensor_msgs
  hbm_img_msgs
  rclcpp_components
)

target_link_libraries(robot_camera
  ${OpenCV_LIBRARIES}
  yaml-cpp
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

ament_package()
