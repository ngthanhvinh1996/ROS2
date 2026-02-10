# Copyright (c) 2024，D-Robotics.
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0077 NEW)
project(mipi_cam_local)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(OpenCV)
   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV > 4.4 not found.")
   endif()

set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
add_compile_options(-w)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fsee -fomit-frame-pointer -fno-signed-zeros -fno-math-errno -funroll-loops")
set(CMAKE_BUILD_TYPE "Release")

## pkg-config libraries
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(yaml_cpp_vendor REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(hbm_img_msgs REQUIRED)

if(PLATFORM_X3)
  message("build platform X3")
  add_definitions(-DPLATFORM_X3)
  set(PREFIX_PATH x3)
  set(SRC_PLATFORM_FILE 
    src/x3/x3_sdk_swap/src/x3_sdk_wrap.cpp
    src/x3/x3_sdk_swap/src/x3_vio_bind.cpp
    src/x3/x3_sdk_swap/src/x3_vio_vin.cpp
    src/x3/x3_sdk_swap/src/x3_vio_vps.cpp
    src/x3/x3_sdk_swap/src/x3_vio_vp.cpp
    src/x3/sensor/sensor_f37_config.c
    src/x3/sensor/sensor_gc4c33_config.c
    src/x3/sensor/sensor_gc4663_config.c
    src/x3/sensor/sensor_imx415_config.c
    src/x3/sensor/sensor_imx586_config.c
    src/x3/sensor/sensor_imx219_config.c
    src/x3/sensor/sensor_imx477_config.c
    src/x3/sensor/sensor_ov5647_config.c
    src/x3/sensor/sensor_sc230ai_config.c
    src/x3/sensor/x3_preparam.c
  )
  include_directories(include 
    src/x3/x3_sdk_swap/include
    src/x3/sensor
  )
elseif(PLATFORM_Rdkultra)
  message("build platform rdkultra")
  add_definitions(-DPLATFORM_Rdkultra)
  set(PREFIX_PATH rdkultra)
  include_directories(${CMAKE_SYSROOT}/usr/include/cam
    ${CMAKE_SYSROOT}/usr/include/isp
    ${CMAKE_SYSROOT}/usr/include/vps/vpm
  )
elseif(PLATFORM_X5)
  message("build platform X5")
  add_definitions(-DPLATFORM_X5)
  set(PREFIX_PATH x5)
  set(SRC_PLATFORM_FILE 
    src/x5/sensor/ov5647_linear_1920x1080_raw10_30fps_2lane.c
    src/x5/sensor/sc132gs_linear_1088x1280_raw10_30fps_1lane.c
    src/x5/sensor/sc230ai_linear_1920x1080_raw10_30fps_1lane.c
    src/x5/sensor/imx219_linear_1920x1080_raw10_30fps_2lane.c
    src/x5/sensor/imx477_linear_1920x1080_raw12_50fps_2lane.c
    src/x5/sensor/imx415_linear_3840x2160_raw10_30fps_2lane.c
    src/x5/sensor/vp_sensors.c
  )
  include_directories(include 
    src/x5/sensor
  )
elseif(PLATFORM_S100)
  message("build platform S100")
  add_definitions(-DPLATFORM_S100)
  set(PREFIX_PATH s100)
  set(SRC_PLATFORM_FILE 
    src/s100/sensor/imx219_linear_1920x1080_raw10_30fps_2lane.c
    src/s100/sensor/sc132gs_linear_1088x1280_raw10_30fps_1lane.c
    src/s100/sensor/sc230ai_linear_1920x1080_raw10_30fps_1lane.c
    src/s100/sensor/ar0820c_linear_1920x1080_yuv_30fps_1lane.c
    src/s100/sensor/ovx3c_linear_1920x1080_raw12_30fps_1lane.c
    src/s100/sensor/vp_sensors.c
  )
  include_directories(include 
    src/s100/sensor
    ${CMAKE_SYSROOT}/usr/include/cjson
  )
elseif(PLATFORM_S600)
  message("build platform S600")
  add_definitions(-DPLATFORM_S600)
  set(PREFIX_PATH s600)
  set(SRC_PLATFORM_FILE 
    src/s600/sensor/imx219_linear_1920x1080_raw10_30fps_2lane.c
    src/s600/sensor/sc132gs_linear_1088x1280_raw10_30fps_1lane.c
    src/s600/sensor/sc230ai_linear_1920x1080_raw10_30fps_1lane.c
    src/s100/sensor/ar0820c_linear_1920x1080_yuv_30fps_1lane.c
    src/s100/sensor/ovx3c_linear_1920x1080_raw12_30fps_1lane.c
    src/s600/sensor/vp_sensors.c
  )
  include_directories(include 
    src/s600/sensor
    ${CMAKE_SYSROOT}/usr/include/cjson
  )
else()
  message("invalid platform, build platform X5 default")
  add_definitions(-DPLATFORM_X5)
  set(PREFIX_PATH x5)
endif()

message("PREFIX_PATH: ${PREFIX_PATH}")

include_directories(include
  include/${PREFIX_PATH}
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_SYSROOT}/usr/
  ${CMAKE_SYSROOT}/usr/include
  ${CMAKE_SYSROOT}/usr/hobot/include
  deps/jsoncpp/include/
)

link_directories(
  ${CMAKE_SYSROOT}/usr/lib
  ${CMAKE_SYSROOT}/usr/hobot/lib
  ${CMAKE_SYSROOT}/usr/lib/hbmedia
  ${CMAKE_SYSROOT}/usr/lib/system/lib
  deps/jsoncpp/lib
)

set(BASE_LIBRARIES pthread dl)
if(PLATFORM_X3)
  set(X3_HAPI_LIB isp_algo z hbmedia isp ion multimedia iar tinyalsa cjson diag gdcbin  jsoncpp)
elseif(PLATFORM_Rdkultra)
  set(X3_HAPI_LIB jsoncpp hb_api_isp vio)
elseif(PLATFORM_X5)
  set(X3_HAPI_LIB jsoncpp multimedia hbmem cam vpf  Nano2D Nano2Dutil)
elseif(PLATFORM_S100)
  set(X3_HAPI_LIB jsoncpp cjson cam vpf)
elseif(PLATFORM_S600)
  set(X3_HAPI_LIB jsoncpp cjson cam vpf)
else()
  set(BASE_LIBRARIES pthread vio cam dl)
  set(X3_HAPI_LIB isp_algo z hbmedia isp ion multimedia iar tinyalsa cjson diag gdcbin)
endif()

# ===============================================
# 1. BUILD SHARED LIB (mipi_cam_local_lib)
# ===============================================
add_library(mipi_cam_local_lib SHARED
  src/hobot_mipi_cam.cpp
  src/${PREFIX_PATH}/hobot_mipi_cap_iml.cpp
  src/${PREFIX_PATH}/hobot_mipi_factory.cpp
  src/camera.cpp
  src/hobot_mipi_node.cpp 
  ${SRC_PLATFORM_FILE}
)

ament_target_dependencies(mipi_cam_local_lib
  rclcpp
  sensor_msgs
  hbm_img_msgs
  rclcpp_components
  OpenCV
)

target_link_libraries(mipi_cam_local_lib
  ${BASE_LIBRARIES} 
  ${X3_HAPI_LIB} 
  yaml-cpp
)

add_library(${PROJECT_NAME}_plugin SHARED
  src/camera.cpp
  src/hobot_mipi_node.cpp
  src/hobot_mipi_cam.cpp
  src/${PREFIX_PATH}/hobot_mipi_cap_iml.cpp
  src/${PREFIX_PATH}/hobot_mipi_factory.cpp
  ${SRC_PLATFORM_FILE}
)

target_link_libraries(${PROJECT_NAME}_plugin
  ${BASE_LIBRARIES} ${X3_HAPI_LIB} yaml-cpp
)

ament_target_dependencies(
  ${PROJECT_NAME}_plugin
  rclcpp
  rclcpp_components
  sensor_msgs
  yaml_cpp_vendor
  hbm_img_msgs
  OpenCV
)

rclcpp_components_register_node(${PROJECT_NAME}_plugin
  PLUGIN "mipi_cam::MipiCamNode"
  EXECUTABLE ${PROJECT_NAME}
)
set(node_plugins "${node_plugins}mipi_cam::MipiCamNode;$<TARGET_FILE:${PROJECT_NAME}_plugin>\n")
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_plugin)

# ===============================================
# 3. EXPORT & INSTALL
# ===============================================
ament_export_include_directories(include) 
# -------------------------------------------------------

ament_export_targets(export_mipi_cam_local_lib HAS_LIBRARY_TARGET)
ament_export_dependencies(
  rclcpp 
  sensor_msgs 
  hbm_img_msgs
  rclcpp_components
  OpenCV
)

# Install executable
install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# Install plugin
install(
  TARGETS ${PROJECT_NAME}_plugin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install shared lib
install(TARGETS mipi_cam_local_lib
  EXPORT export_mipi_cam_local_lib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

if(PLATFORM_X3)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/x3/ DESTINATION lib/${PROJECT_NAME}/config/)
elseif(PLATFORM_Rdkultra)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/rdkultra/ DESTINATION lib/${PROJECT_NAME}/config/)
elseif(PLATFORM_X5)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/x5/ DESTINATION lib/${PROJECT_NAME}/config/)
elseif(PLATFORM_S100)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/s100/ DESTINATION lib/${PROJECT_NAME}/config/)
elseif(PLATFORM_S600)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/s600/ DESTINATION lib/${PROJECT_NAME}/config/)
endif()

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME}/)

# Copy header files vào thư mục install
install(DIRECTORY include/ DESTINATION include)

install(DIRECTORY src/${PREFIX_PATH}/sensor/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

ament_package()