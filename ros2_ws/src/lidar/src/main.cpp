#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "lidar_ros2.hpp"

using lidar::LidarRos2;

int main(int argc, char** argv)
{
    RCLCPP_INFO(rclcpp::get_logger("lidar"), "Lidar is working");

    rclcpp::init(argc, argv);
    rclcpp::NodeOptions opt;
    auto node = std::make_shared<LidarRos2>(opt);
    // rclcpp::executors::SingleThreadedExecutor exec;
    // exec.add_node(node);
    // exec.spin();

    while(rclcpp::ok() && node->processing_lidar())
    {
        rclcpp::spin_some(node);
    }

    rclcpp::shutdown();
    return 0;
}