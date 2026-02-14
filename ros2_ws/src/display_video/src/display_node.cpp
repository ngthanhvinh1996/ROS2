#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"

class DisplayNode : public rclcpp::Node
{
public:
  DisplayNode() : Node("display_video_node")
  {
    rclcpp::QoS qos_profile(10);
    qos_profile.reliability(rclcpp::ReliabilityPolicy::BestEffort);

    subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
      "image_raw",
      qos_profile,
      std::bind(&DisplayNode::topic_callback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Display Node has been started. Waiting for images...");
    
    cv::namedWindow("Robot Camera Stream", cv::WINDOW_AUTOSIZE);
  }

private:
  void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    cv::Mat display_frame;

    try {
      if (msg->encoding == "nv12") {
        cv::Mat nv12_mat(msg->height * 3 / 2, msg->width, CV_8UC1, const_cast<uint8_t*>(msg->data.data()));

        cv::cvtColor(nv12_mat, display_frame, cv::COLOR_YUV2BGR_NV12);
      } 
      else {
        display_frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
      }

      if (display_frame.empty()) {
        RCLCPP_WARN(this->get_logger(), "Frame is empty!");
        return;
      }

      cv::imshow("Robot Camera Stream", display_frame);
      cv::waitKey(1);

      static int count = 0;
      if (++count % 60 == 0) {
        RCLCPP_INFO(this->get_logger(), "Displaying: %dx%d (%s)", 
                    msg->width, msg->height, msg->encoding.c_str());
      }

    } catch (cv::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
    } catch (cv_bridge::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge Error: %s", e.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DisplayNode>());
  rclcpp::shutdown();
  return 0;
}