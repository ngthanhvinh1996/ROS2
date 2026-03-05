#ifndef ROBOT_CAM_NODE_HPP_
#define ROBOT_CAM_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <std_msgs/msg/string.hpp>
#include "robot_camera.hpp" 
#include "x5/hobot_mipi_cap_iml.hpp"
#include "hobot_mipi_comm.hpp"
#include "hobot_mipi_cam.hpp"

namespace robot_cam
{
typeded struct Publisher_info_base {
    sensor_msgs::msg::CameraInfo::UniquePtr camera_calibration_info_;
    sensor_msgs::msg::CameraInfo::UniquePtr camera_calibration_info2_;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr info_pub_ = nullptr;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr info_pub2_ = nullptr;
    std::chrono::time_point<std::chrono::system_clock> time_start_;
}

typedef struct Publisher_info : Publisher_info_base {
    sensor_msgs::msg::Image::UniquePtr img_;
    std::string topic_type;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_ = nullptr;
} Publisher_info_st;

typedef struct Publisher_hbmem_info : Publisher_info_base {
    int mSendIdx;
    rclcpp::Publisher<hbm_img_msgs::msg::HbmMsg1080P>::SharedPtr publisher_hbmem_;
    std::string topic_type;
}

class RobotCameraNode : public rclcpp::Node {
public:
    RobotCameraNode(const rclcpp::NodeOptions & node_options);
    ~RobotCameraNode();
    void init();
    void update(Publisher_info_st* pub_info);
    void hbmemUpdate(Publisher_hbmem_info_st* pub_info);
private:
    void init_publisher(Publisher_info_st &Pub_info, std::string topic, std::string topic_type, std::string frame_id);
    void init_publisher_hbmem(Publisher_hbmem_info_st &Pub_info, std::string topic, std::string topic_type);
    std::shared_ptr<RobotCamera> robotCam_ptr_;
    std::vector<std::shared_ptr<std::thread>> timer_;
    rclcpp::TimerBase::SharedPtr timer_tmp_;
    int m_bIsInit;
    sensor_msgs::msg::CameraInfo::UniquePtr camera_calibration_info_;
    std::shared_ptr<struct mipi_cam::NodePara> nodePara_;
    std::string frame_id_;
    std::string io_method_name_;
    std::vector<Publisher_info_st> Pub_info_;
};

} // robot_cam

#endif /* ROBOT_CAM_NODE_HPP_ */