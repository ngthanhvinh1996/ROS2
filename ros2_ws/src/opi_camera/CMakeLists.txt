cmake_minimum_required(VERSION 3.8)
project(opi_camera)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(opi_camera_node src/camera_node.cpp)

target_include_directories(opi_camera_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${OpenCV_INCLUDE_DIRS}
)

ament_target_dependencies(opi_camera_node
  rclcpp
  sensor_msgs
  cv_bridge
  image_transport
)

target_link_libraries(opi_camera_node ${OpenCV_LIBRARIES})

install(TARGETS opi_camera_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()