#ifndef LIDAR_ROS2_HPP_
#define LIDAR_ROS2_HPP_

#include "lidar_types.hpp"
#include "lidar_driver.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <cmath>
#include <limits>

namespace lidar {

class LidarRos2 : public rclcpp::Node {
public:
    LidarRos2(const rclcpp::NodeOptions& options);
    ~LidarRos2();
    bool processing_lidar();

private:
    void load_parameters();
    uint8_t calculate_crc8(const unsigned char *data, size_t len);
    void parse_packet(unsigned char *data);
    bool feed_data(unsigned char *data, size_t len);

    constexpr static size_t PACKET_SIZE = 108;
    std::unique_ptr<LidarDriver> driver_;
    LidarConfig config_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
    std::vector<ScanData> scan_data_;
    std::vector<ScanData> scan_data_processing_;
    std::vector<ScanPoint> scan_points_;
    unsigned char *data_;
    bool first_time_flag_;
    uint32_t idx_;
    uint32_t idx_processing_;
};

}

#endif /* LIDAR_ROS2_HPP_ */