cmake_minimum_required(VERSION 3.8)
project(display_video)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Tìm các gói phụ thuộc
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)

# Tạo Executable
add_executable(display_video_node src/display_node.cpp)

# Link thư viện
ament_target_dependencies(display_video_node
  rclcpp
  sensor_msgs
  cv_bridge
  OpenCV
)

# Cài đặt (Install)
install(TARGETS
  display_video_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()