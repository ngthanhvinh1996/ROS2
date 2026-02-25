#include <cstdio>
#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/header.hpp"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.hpp"
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;

class OpiCameraNode : public rclcpp::Node {
public:
    OpiCameraNode() : Node("opi_camera_node") {
        publisher_ = image_transport::create_publisher(this, "camera/image_raw");

        std::string pipeline = "v4l2src device=/dev/video11 ! "
                               "video/x-raw, format=NV12, width=800, height=600 ! "
                               "videoconvert ! "
                               "video/x-raw, format=BGR ! "
                               "appsink drop=1";

        RCLCPP_INFO(this->get_logger(), "Opening Camera via GStreamer...");
        cap_.open(pipeline, cv::CAP_GSTREAMER);

        if (!cap_.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "Cannot open Camera! Please check /dev/video11");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Camera is already! publish on topic /camera/image_raw");

        timer_ = this->create_wall_timer(
            33ms, std::bind(&OpiCameraNode::timer_callback, this));
    }

private:
    void timer_callback() {
        cv::Mat frame;

        cap_ >> frame;

        if (frame.empty()) {
            RCLCPP_WARN(this->get_logger(), "Frame empty (Dropped frame)");
            return;
        }

        std_msgs::msg::Header header;
        header.stamp = this->get_clock()->now();
        header.frame_id = "camera_link";

        sensor_msgs::msg::Image::SharedPtr msg = cv_bridge::CvImage(
            header, 
            "bgr8",
            frame
        ).toImageMsg();

        publisher_.publish(*msg);
    }

    image_transport::Publisher publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<OpiCameraNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}