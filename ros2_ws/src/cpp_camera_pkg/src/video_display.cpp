#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"

class VideoDisplay : public rclcpp::Node
{
public:
  VideoDisplay() : Node("video_display_node")
  {
    cv::namedWindow("ROS 2 Camera Stream", cv::WINDOW_AUTOSIZE);

    subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
      // "camera/image_raw", 
      "/image_left_raw",
      10, 
      std::bind(&VideoDisplay::topic_callback, this, std::placeholders::_1));
      
    RCLCPP_INFO(this->get_logger(), "Dang cho du lieu video...");
  }

  ~VideoDisplay()
  {
    cv::destroyAllWindows();
  }

private:
  void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    try {
      cv::Mat frame = cv_bridge::toCvCopy(msg, "bgr8")->image;

      cv::imshow("ROS 2 Camera Stream", frame);
      
      cv::waitKey(1); 
    } 
    catch (cv_bridge::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Loi cv_bridge: %s", e.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VideoDisplay>());
  rclcpp::shutdown();
  return 0;
}
