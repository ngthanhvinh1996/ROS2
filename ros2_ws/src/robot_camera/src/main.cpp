#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "robot_camera_node.hpp"

using robot_cam::RobotCameraNode;

int main(int argc, char** argv)
{
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "This is version for optimizing camera timestamp!");

    rclcpp::init(argc, argv);
    rclcpp::NodeOptions opt;
    auto node = std::make_shared<RobotCamNode>(opt);
    rclcpp::executors::SingleThreadedExecutor exec;
    exec.add_node(node);
    exec.spin();

    rclcpp::shutdown();
    return 0;
}
