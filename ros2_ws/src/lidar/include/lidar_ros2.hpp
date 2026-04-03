#ifndef LIDAR_ROS2_HPP_
#define LIDAR_ROS2_HPP_

#include "lidar_types.hpp"
#include "lidar_driver.hpp"
#include "lidar_parser.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

namespace lidar {

class LidarRos2 : public rclcpp::Node {
public:
    LidarRos2(const rclcpp::NodeOptions& options);
    ~LidarRos2();

private:
    void timer_callback();
    void publish_data(const ScanData& data);
    void declare_parameters();

    std::unique_ptr<LidarDriver> driver_;
    std::unique_ptr<LidarParser> parser_;
    LidarConfig config_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
};

}

#endif /* LIDAR_ROS2_HPP_ */