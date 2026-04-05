#ifndef LIDAR_ROS2_HPP_
#define LIDAR_ROS2_HPP_

#include "lidar_types.hpp"
#include "lidar_driver.hpp"
// TODO: uncomment when lidar_parser.hpp is created
// #include "lidar_parser.hpp"
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
    void load_parameters();
    void parse_packet(const std::vector<uint8_t>& data, size_t len);
    void feed_data(std::vector<uint8_t>& data, size_t len);

    std::unique_ptr<LidarDriver> driver_;
    LidarConfig config_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
    std::vector<ScanPoint> scan_points_;
    ScanData scan_data_;
    std::vector<uint8_t> data_;
};

}

#endif /* LIDAR_ROS2_HPP_ */