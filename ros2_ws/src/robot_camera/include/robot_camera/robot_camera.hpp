#ifndef ROBOT_CAM_HPP_
#define ROBOT_CAM_HPP_

#include "rclcpp/rclcpp.hpp"
#include "hobot_mipi_node.hpp" 
#include "x5/hobot_mipi_cap_iml.hpp"
#include "x5/hobot_mipi_factory.hpp"
#include "hobot_mipi_comm.hpp"
#include <opencv2/opencv.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <sensor_msgs/distortion_models.hpp>

namespace robot_cam
{
    
class RobotCamera {
public:
    RobotCamera() = default;
    virtual ~RobotCamera() = default;
    static std::shared_ptr<RobotCamera> create_camera();
    virtual int init(std::shared_ptr<struct mipi_cam::NodePara> para) = 0;
    virtual int deInit() = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual bool getImage(builtin_interfaces::msg::Time &stamp,
                             std::string &endcoding,
                             uint32_t &height,
                             uint32_t &width,
                             uint32_t &step,
                             std::vector<uint8_t> &data, std::string channel) = 0;
    virtual bool getImageMem(builtin_interfaces::msg::Time &stamp,
                             std::array<uint8_t, 12> &encoding,
                             uint32_t &height,
                             uint32_t &width,
                             uint32_t &step,
                             std::array<uint8_t, 6220800> &data,
                             uint32_t &data_size,
                             std::string channel) = 0;
    virtual bool getCamCalibration(sensor_msgs::msg::CameraInfo& cam_info,
                           const std::string &file_path) = 0;
    virtual bool getDualCamCalibration(sensor_msgs::msg::CameraInfo &cam_info_l,
                sensor_msgs::msg::CameraInfo &cam_info_r, const std::string &file_path) = 0;
    // virtual bool getCombineImage() = 0;
    virtual bool isCapturing() = 0;
};

} // robot_cam

#endif /* ROBOT_CAM_HPP_ */