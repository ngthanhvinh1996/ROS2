#ifndef ROBOT_CAM_NODE_HPP_
#define ROBOT_CAM_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <std_msgs/msg/string.hpp>
#include "robot_cam_node.hpp" 
#include "robot_cam.hpp" 
#include "x5/hobot_mipi_cap_iml.hpp"

namespace robot_cam
{

typedef struct Publisher_info {
    sensor_msgs::msg::CameraInfo::UniquePtr img_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr img_pub_;
    std::string topic_type;
} Publisher_info_st;

class RobotCameraNode : public rclcpp::Node {
public:
    RobotCameraNode(const rclcpp::NodeOptions & node_options);
    ~RobotCameraNode();
    void init();
    void update(Publisher_info_st* pub_info);
private:
    void init_publisher(Publisher_info_st &Pub_info, std::string topic, std::string topic_type, std::string frame_id);
    std::share_ptr<RobotCam> robotCam_ptr_;
    std::vector<std::shared_ptr<std::thread>> timer_;
    rclcpp::TimerBase::SharedPtr timer_tmp_;
    int m_bIsInit;
    sensor_msgs::msg::CameraInfo::UniquePtr camera_calibration_info_;
    std::shared_ptr<struct NodePara> nodePara_;
    std::string frame_id_;
}

} // robot_cam

#endif /* ROBOT_CAM_NODE_HPP_ */