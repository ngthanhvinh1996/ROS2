#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include "cpp_camera_pkg/msg/num.hpp"

using namespace std::chrono_literals;


class CameraPublisher : public rclcpp::Node
{
public:
  CameraPublisher()
  : Node("camera_node")
  {
    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("camera/image_raw", 10);

    int device_id = 0; 
    cap_.open(device_id, cv::CAP_V4L2);

    if (!cap_.isOpened()) {
      RCLCPP_ERROR(this->get_logger(), "Cannot open USB Camera %d!", device_id);
      return;
    }

    cap_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    cap_.set(cv::CAP_PROP_FPS, 30);

    double fps = cap_.get(cv::CAP_PROP_FPS);
    RCLCPP_INFO(this->get_logger(), "USB Camera initialize failed! FPS: %.2f", fps);

    timer_ = this->create_wall_timer(
      33ms, std::bind(&CameraPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    cv::Mat frame;
    cap_ >> frame;

    if (frame.empty()) {
      RCLCPP_WARN(this->get_logger(), "Empty frame");
      return;
    }

  //  cv:imshow("Camera Preview", frame);
  //  cv::waitKey(1);

    std_msgs::msg::Header header;
    header.stamp = this->get_clock()->now();
    header.frame_id = "camera_link";

    sensor_msgs::msg::Image::SharedPtr msg = cv_bridge::CvImage(
      header, 
      "bgr8",
      frame
    ).toImageMsg();

    // Publish
    publisher_->publish(*msg);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
  cv::VideoCapture cap_;
};

class MinimalPublisher : public rclcpp::Node
{
public:
  MinimalPublisher() : Node("minimal_publisher")
  {
    publisher = this->create_publisher<cpp_camera_pkg::msg::Num>("topic", 10);
    timer_ = this->create_wall_timer(500ms, std::bind(&MinimalPublisher::timer_callback, this));
  }
private:
  void timer_callback()
  {
    auto message = cpp_camera_pkg::msg::Num();
    message.num = 10;
    RCLCPP_INFO_STREAM(this->get_logger(), "Pushlishing: '" << message.num << "'");
    publisher->publish(message);
  }
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<cpp_camera_pkg::msg::Num>::SharedPtr publisher;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CameraPublisher>());
  // rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}
